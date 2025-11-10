#include "cuphead_data_generator.h"

#include <zmq.hpp>
#include <nlohmann/json.hpp>

#include "constants.h"
#include "cuphead_frame.h"
#include "debug_utils.h"
#include "hit_flash_detector.h"

namespace {

void draw_detections(cv::Mat& img, const std::vector<EntityDetection>& detections) {
    for (const auto& [box, type, conf] : detections) {
        const bool is_player = (type == EntityType::player);
        cv::Scalar col = is_player ? constants::green_scalar : constants::orange_scalar;
        cv::rectangle(img, box, col, 2, cv::LINE_AA);

        const char* name = is_player ? "Player" : "Boss";
        std::string text = cv::format("%s %.2f", name, conf);
        int base = 0; auto sz = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &base);
        cv::rectangle(img, cv::Rect(box.x, box.y - sz.height - 6, sz.width + 6, sz.height + 6),
            col, cv::FILLED);
        cv::putText(img, text, { box.x + 3, box.y - 4 },
            cv::FONT_HERSHEY_SIMPLEX, 0.5, { 0,0,0 }, 1, cv::LINE_AA);
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

    constexpr auto k_hp_period = 250ms;
    auto last_hp_infer = clock::now() - k_hp_period;
    HitFlashDetector boss_flash;

    // --- Shared frame buffer ---
    std::mutex mtx;
    cv::Mat latest_frame;
    std::atomic<bool> running{ true };
    std::condition_variable cv_frame_ready;
    bool frame_ready = false;

    // --- Thread A: Capture frames ---
    std::thread capture_thread([&]() {
        while (running) {
            cv::Mat frame = window_capture_->Capture();
            if (frame.empty()) continue;
            {
                std::lock_guard<std::mutex> lock(mtx);
                frame.copyTo(latest_frame);
                frame_ready = true;
            }
            cv_frame_ready.notify_one();
        }
        });

    // --- Thread B: Processing & display ---
    while (true) {
        cv::Mat frame;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv_frame_ready.wait(lock, [&] { return frame_ready || !running; });
            if (!running) break;
            latest_frame.copyTo(frame);
            frame_ready = false;
        }

        if (frame.empty()) {
            continue;
        }

        const double smoothed_fps = fps_tracker_.UpdateFps();
        cv::Mat visual = frame.clone();
        cv::putText(visual, cv::format("FPS: %.1f", smoothed_fps),
            { 10, 20 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, constants::black_scalar, 2);

        std::vector<EntityDetection> entities = RunEntityDetection(frame);
        const cv::Rect hp_rect = CupheadFrame(frame).FindHpBadge();

        if (const auto now = clock::now(); now - last_hp_infer >= k_hp_period) {
            last_hp_infer = now;
            if (auto hp = ClassifyPlayerHp(frame, hp_rect))
                player_hp_ = *hp;
        }

        cv::Rect boss_box; int best_area = 0;
        for (const auto& e : entities) {
            if (e.type == EntityType::boss && e.box.area() > best_area) {
                best_area = e.box.area();
                boss_box = e.box;
            }
        }

        if (best_area > 0) {
            cv::Rect safe = boss_box & cv::Rect(0, 0, frame.cols, frame.rows);
            if (safe.area() > 0 && boss_flash.Update(frame(safe))) {
                cv::putText(visual, "HIT!", { 10, 45 },
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, constants::black_scalar, 2);
            }
        }

        cv::rectangle(visual, hp_rect, constants::yellow_scalar, 2);
        cv::putText(visual, "HP: " + std::to_string(player_hp_),
            { hp_rect.x, std::max(0, hp_rect.y - 5) },
            cv::FONT_HERSHEY_SIMPLEX, 0.4, constants::yellow_scalar, 1);
        draw_detections(visual, entities);

        cv::imshow(preview_window, visual);

        if (cv::waitKey(1) == constants::esc_key) {
            running = false;
            cv_frame_ready.notify_all();
            break;
        }
    }

    running = false;
    cv_frame_ready.notify_all();
    if (capture_thread.joinable()) capture_thread.join();

    cv::destroyAllWindows();
}



void CupheadDataGenerator::StreamData() const {
	zmq::context_t context(1);
	zmq::socket_t publisher(context, zmq::socket_type::pub);
	publisher.set(zmq::sockopt::sndhwm, 1);
	publisher.bind("tcp://*:5555");

	CupheadFrame prev_frame(window_capture_->Capture());
	while (true) {
		CupheadFrame current_frame(window_capture_->Capture());
		if (cv::waitKey(30) == constants::esc_key) break;
		if (current_frame.Empty()) continue;

		// Serialize
		nlohmann::json j;
		std::string serialized_data = j.dump();

		// Send
		zmq::message_t message(serialized_data.begin(), serialized_data.end());
		publisher.send(message, zmq::send_flags::none);

		std::swap(prev_frame, current_frame);
	}
}
