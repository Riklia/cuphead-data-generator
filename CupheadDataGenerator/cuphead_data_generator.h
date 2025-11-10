#pragma once

#include <memory>

#include "cuphead_entity_detector.h"
#include "cuphead_hp_classifier.h"
#include "window_capture.h"

// Provides an interface for receiving data from the game.
class CupheadDataGenerator {

    class SmoothedFpsTracker {
    public:
        double UpdateFps();

    private:
        std::deque<double> frame_times_;
        std::chrono::time_point<std::chrono::steady_clock> last_time_ = std::chrono::steady_clock::now();
    };

public:
    explicit CupheadDataGenerator(const HWND& window_handle)
        : window_capture_(std::make_unique<WindowCapture>(window_handle)) {
	}

    // Initial idea: we could have a list of all functions needed in data and
    // then call it in one function CaptureFrameData. We need to capture all the
    // data from the same frame. For example, we should not return HP from one frame,
    // but Stamina from another one.

    bool IsValid() const { return window_capture_->IsValid(); }

    void PreviewStreamData();

    void StreamData() const;

private:
    std::vector<EntityDetection> RunEntityDetection(const cv::Mat& frame);

    std::optional<int> ClassifyPlayerHp(const cv::Mat& frame, const cv::Rect& hp_rect) const;

    int player_hp_ = 0;

    std::unique_ptr<WindowCapture> window_capture_;
    HpClassifier player_hp_classifier_{ "models/hp_tf.onnx", /*img_size=*/32 };
    CupheadEntityDetector entity_detector_{ L"models/boss_player_bb_320.onnx", 320 };

    SmoothedFpsTracker fps_tracker_;

};
