#include "cuphead_data_generator.h"

#include <zmq.hpp>
#include <nlohmann/json.hpp>

#include "constants.h"
#include "ex_meter_detector.h"
#include "hit_flash_detector.h"

namespace {

cv::Scalar ColorForEntity(EntityType type) {
    switch (type) {
    case EntityType::player:
        return constants::green_scalar;
    case EntityType::boss:
        return constants::orange_scalar;
    case EntityType::projectile:
        return constants::red_scalar;
    case EntityType::parryable:
        return constants::pink_scalar;
    default:
        return constants::grey_scalar;
    }
}


void DrawDetections(cv::Mat& img, const std::vector<EntityDetection>& detections) {
    for (const auto& [box, entity_type, conf] : detections) {

        cv::Scalar col = ColorForEntity(entity_type);
        const char* name = ToString(entity_type);

        cv::rectangle(img, box, col, 2, cv::LINE_AA);
        std::string text = cv::format("%s %.2f", name, conf);
        int base = 0;
        const auto sz = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &base);

        const int text_y = std::max(0, box.y - sz.height - 6);
        cv::rectangle(img,
            cv::Rect(box.x, text_y, sz.width + 6, sz.height + 6),
            col, cv::FILLED);

        cv::putText(img, text,
            { box.x + 3, text_y + sz.height + 2 },
            cv::FONT_HERSHEY_SIMPLEX, 0.5, { 0, 0, 0 }, 1, cv::LINE_AA);
    }
}

}  // namespace


std::vector<EntityDetection> CupheadDataGenerator::RunEntityDetection(const cv::Mat& frame) {
    return entity_detector_.DetectEntities(frame, 0.6f, 0.5f);
}

std::optional<int> CupheadDataGenerator::ClassifyPlayerHp(const cv::Mat& frame, const cv::Rect& hp_rect) const {
    cv::Rect safe = hp_rect & cv::Rect(0, 0, frame.cols, frame.rows);
    if (safe.width == 0 || safe.height == 0) return std::nullopt;
    return player_hp_classifier_.classify(frame(safe), 0.90f);
}

double CupheadDataGenerator::SmoothedFpsTracker::UpdateFps() {
	const auto now = std::chrono::steady_clock::now();
	const double elapsed = std::chrono::duration<double>(now - last_time_).count();
	last_time_ = now;

	frame_times_.push_back(elapsed);
	if (frame_times_.size() > constants::fps_window) {
		frame_times_.pop_front();
	}

	const double mean_dt = std::accumulate(frame_times_.begin(), frame_times_.end(), 0.0) / frame_times_.size();
	return 1.0 / std::max(mean_dt, 1e-6);
}

void CupheadDataGenerator::PreviewStreamData() {
    using clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    const std::string preview_window = "Preview";
    cv::namedWindow(preview_window, cv::WINDOW_AUTOSIZE);

    FrameCaptureWorker cap(window_capture_.get());
    cap.Start();

    constexpr auto hp_inference_freq = 250ms;
    auto last_hp_infer = clock::now() - hp_inference_freq;

    while (true) {
        cv::Mat frame;
        if (!cap.WaitAndGet(frame)) {
            break;
        }

        const double fps = fps_tracker_.UpdateFps();
        cv::Mat visual = frame.clone();
        cv::putText(visual, cv::format("FPS: %.1f", fps),
            { 10, 20 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, constants::black_scalar, 2);

        std::vector<EntityDetection> entities = RunEntityDetection(frame);

        if (const auto now = clock::now(); now - last_hp_infer >= hp_inference_freq) {
            last_hp_infer = now;
            if (auto hp = ClassifyPlayerHp(frame, constants::hp_badge)) {
                player_hp_ = *hp;
            }
        }
        cv::rectangle(visual, constants::hp_badge, constants::yellow_scalar, 2);
        cv::putText(visual, "HP: " + std::to_string(player_hp_),
            { constants::hp_badge.x, std::max(0, constants::hp_badge.y - 5) },
            cv::FONT_HERSHEY_SIMPLEX, 0.4, constants::yellow_scalar, 1);

        const int ex_cards = ex_detector_.FindExValue(frame, &visual);
        cv::putText(visual, cv::format("EX: %d/5", ex_cards),
            { 10, 40 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, constants::black_scalar, 2);

        cv::Rect boss_box;
    	int best_area = 0;
        for (const auto& entity : entities) {
            if (entity.type == EntityType::boss && entity.box.area() > best_area) {
                best_area = (boss_box = entity.box).area();
            }
        }
        if (best_area > 0) {
            cv::Rect safe = boss_box & cv::Rect(0, 0, frame.cols, frame.rows);
            if (safe.area() > 0 && hit_detector_.Update(frame(safe))) {
                cv::putText(visual, "HIT!", { 10, 70 }, cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    constants::black_scalar, 2);
            }
        }
        DrawDetections(visual, entities);

        cv::imshow(preview_window, visual);
        if (cv::waitKey(1) == constants::esc_key) {
            break;
        }
    }

    cap.Stop();
    cv::destroyAllWindows();
}

void CupheadDataGenerator::StreamData() const {
    /*
	zmq::context_t context(1);
	zmq::socket_t publisher(context, zmq::socket_type::pub);
	publisher.set(zmq::sockopt::sndhwm, 1);
	publisher.bind("tcp://*:5555");

	CupheadFrame prev_frame(window_capture_->Capture());
	while (true) {
		CupheadFrame current_frame(window_capture_->Capture());
        if (cv::waitKey(30) == constants::esc_key) {
            break;
        }
        if (current_frame.Empty()) {
            continue;
        }

		// Serialize
		nlohmann::json j;
		std::string serialized_data = j.dump();

		// Send
		zmq::message_t message(serialized_data.begin(), serialized_data.end());
		publisher.send(message, zmq::send_flags::none);

		std::swap(prev_frame, current_frame);
	}
    */
}
