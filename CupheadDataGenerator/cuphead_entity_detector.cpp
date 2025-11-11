#include "cuphead_entity_detector.h"

#include <algorithm>

namespace {

float Clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

// Resizes the input image to a square, while preserving aspect ratio and adding gray padding
// to match YOLO preprocessing.
cv::Mat ExtractLetterbox(const cv::Mat& img, int new_shape, const cv::Scalar& color,
    bool scaleup, float& gain, int& padw, int& padh) {

    const int width = img.cols;
    const int height = img.rows;

    float r = std::min(static_cast<float>(new_shape) / width, static_cast<float>(new_shape) / height);
    if (!scaleup) r = std::min(r, 1.0f);

    const int nw = static_cast<int>(std::round(width * r));
    const int nh = static_cast<int>(std::round(height * r));

    padw = (new_shape - nw) / 2;
    padh = (new_shape - nh) / 2;

    cv::Mat resized; cv::resize(img, resized, cv::Size(nw, nh));
    cv::Mat out(new_shape, new_shape, CV_8UC3, color);
    resized.copyTo(out(cv::Rect(padw, padh, nw, nh)));

    gain = r;
    return out;
}

// Applies non-maximum suppression to remove overlapping detections, keeping only
// the highest-confidence bounding boxes per object.
void NonMaximumSuppression(std::vector<EntityDetection>& detections, float iou_thresh) {
    std::ranges::sort(detections,
        [](const EntityDetection& a, const EntityDetection& b) { return a.confidence > b.confidence; });

    std::vector<EntityDetection> keep;
    std::vector<char> removed(detections.size(), 0);
    for (size_t i = 0; i < detections.size(); ++i) {
        if (removed[i]) {
            continue;
        }
        keep.push_back(detections[i]);

        const auto& A = detections[i].box;
        for (size_t j = i + 1; j < detections.size(); ++j) {
            if (removed[j]) {
                continue;
            }
            const auto& B = detections[j].box;

            const int xx1 = std::max(A.x, B.x);
            const int yy1 = std::max(A.y, B.y);
            const int xx2 = std::min(A.x + A.width, B.x + B.width);
            const int yy2 = std::min(A.y + A.height, B.y + B.height);
            const int width = std::max(0, xx2 - xx1);
            const int height = std::max(0, yy2 - yy1);
            const float inter = static_cast<float>(width * height);
            const float ua = static_cast<float>(A.width * A.height + B.width * B.height) - inter;
            if (const float iou = ua > 0 ? inter / ua : 0.f; iou > iou_thresh) {
                removed[j] = 1;
            }
        }
    }
    detections.swap(keep);
}

// Splits HWC float image into CHW contiguous buffer.
void HwcToChw(const cv::Mat& img_rgb01, int input_size, std::vector<float>& out_chw) {
    out_chw.resize(3LL * input_size * input_size);
    std::vector<cv::Mat> channels(3);
    for (int i = 0; i < 3; ++i) {
        channels[i] = cv::Mat(input_size, input_size, CV_32F, i * input_size * input_size + out_chw.data());
    }
    cv::split(img_rgb01, channels);
}

// Converts xywh space to xyxy and clamp. Returns false if box is degenerate after rounding.
bool XywhToRectUnletterbox(float cx, float cy, float w, float h,
    float gain, int padw, int padh, int img_w, int img_h, cv::Rect& box_out) {
    float x1 = (cx - w * 0.5f - static_cast<float>(padw)) / gain;
    float y1 = (cy - h * 0.5f - static_cast<float>(padh)) / gain;
    float x2 = (cx + w * 0.5f - static_cast<float>(padw)) / gain;
    float y2 = (cy + h * 0.5f - static_cast<float>(padh)) / gain;

    x1 = Clampf(x1, 0.f, static_cast<float>(img_w - 1));
    y1 = Clampf(y1, 0.f, static_cast<float>(img_h - 1));
    x2 = Clampf(x2, 0.f, static_cast<float>(img_w - 1));
    y2 = Clampf(y2, 0.f, static_cast<float>(img_h - 1));

    box_out = cv::Rect(static_cast<int>(std::round(x1)), static_cast<int>(std::round(y1)),
        static_cast<int>(std::round(x2 - x1)), static_cast<int>(std::round(y2 - y1)));
    return box_out.width > 0 && box_out.height > 0;
}

EntityType IdToEntityType(int id) {
    switch (id) {
    case 0:
        return EntityType::player;
    case 1:
        return EntityType::boss;
    case 2:
        return EntityType::projectile;
    case 3:
        return EntityType::parryable;
    default:
        return EntityType::invalid;
    }
}

}  // namespace


CupheadEntityDetector::CupheadEntityDetector(const std::wstring& onnx_path, int input_size): env_(ORT_LOGGING_LEVEL_WARNING, "cuphead-detector"),
	allocator_(std::make_unique<Ort::AllocatorWithDefaultOptions>()),
	input_size_(input_size) {

	session_opts_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
	session_ = Ort::Session(env_, onnx_path.c_str(), session_opts_);
	meminfo_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

	input_name_ = session_.GetInputNameAllocated(0, *allocator_).get();
	output_name_ = session_.GetOutputNameAllocated(0, *allocator_).get();
}

std::vector<EntityDetection> CupheadEntityDetector::DetectEntities(
    const cv::Mat& bgr_frame, float conf_threshold, float iou_threshold) {

    std::vector<EntityDetection> detections;
    if (bgr_frame.empty()) {
        return detections;
    }

    float gain;
	int padw, padh;
    cv::Mat lb = ExtractLetterbox(bgr_frame, input_size_, { 114,114,114 }, true, gain, padw, padh);
    cv::cvtColor(lb, lb, cv::COLOR_BGR2RGB);
    lb.convertTo(lb, CV_32F, 1.0f / 255.0f);

    const std::vector<int64_t> shape = { 1, 3, input_size_, input_size_ };
    std::vector<float> chw;
    HwcToChw(lb, input_size_, chw);

    const Ort::Value input = Ort::Value::CreateTensor<float>(
        meminfo_, chw.data(), chw.size(), shape.data(), shape.size());

    const char* in_names[] = { input_name_.c_str() };
    const char* out_names[] = { output_name_.c_str() };
    auto outs = session_.Run(Ort::RunOptions{ nullptr }, in_names, &input, 1, out_names, 1);

    auto& out = outs.front();
    const auto out_shape = out.GetTensorTypeAndShapeInfo().GetShape();

    if (out_shape.size() != 3) {
        return detections;
    }

    const int64_t C = out_shape[1];
    const int64_t N = out_shape[2];
    const int nc = static_cast<int>(C - 4);

    const float* out_data = out.GetTensorMutableData<float>();
    detections.reserve(static_cast<size_t>(N));
    auto at_out_data = [&](int c, size_t i) { return out_data[c * N + i]; };

    for (int64_t i = 0; i < N; ++i) {
        const float cx = at_out_data(0, i);
        const float cy = at_out_data(1, i);
        const float w = at_out_data(2, i);
        const float h = at_out_data(3, i);

        int best_id = -1;
    	float best_score = 0.f;

        const bool has_obj = (C == 5 + nc);
        const float obj = has_obj ? at_out_data(4, i) : 1.f;
        const int cls_start = has_obj ? 5 : 4;

        for (int c = 0; c < nc; ++c) {
            float score = obj * at_out_data(cls_start + c, i);
            if (score > best_score) {
	            best_score = score;
            	best_id = c;
            }
        }
        if (best_score < conf_threshold) {
            continue;
        }

        cv::Rect box;
        if (!XywhToRectUnletterbox(cx, cy, w, h, gain, padw, padh,
            bgr_frame.cols, bgr_frame.rows, box)) {
            continue;
        }

        detections.push_back({ .box = box, .type = IdToEntityType(best_id), .confidence = best_score });
    }

    NonMaximumSuppression(detections, iou_threshold);
    return detections;
}
