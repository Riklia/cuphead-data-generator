#include "hit_flash_detector.h"

namespace {

double NowSeconds() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

}  // namespace

double HitFlashDetector::ComputeBrightnessScore(const cv::Mat& bgr) const {
    if (bgr.empty()) {
        return 0.0;
    }
    const cv::Rect full(0, 0, bgr.cols, bgr.rows);

    const int width = static_cast<int>(full.width * center_crop_);
    const int height = static_cast<int>(full.height * center_crop_);
    const int x = (full.width - width) / 2;
    const int y = (full.height - height) / 2;
    cv::Rect core(x, y, width, height);
    core &= full;
    if (core.width <= 2 || core.height <= 2) return 0.0;

    cv::Mat gray;
    cv::cvtColor(bgr(core), gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, { 3, 3 }, 0);
    return cv::mean(gray)[0];
}

bool HitFlashDetector::Update(const cv::Mat& boss_bgr) {
    const double score = ComputeBrightnessScore(boss_bgr);
    const double now_s = NowSeconds();

    if (!initialized_) {
        initialized_ = true;
        ema_ = score;
        ring_ = { score, score, score };
        return false;
    }

    const double delta = score - ema_;
    ema_ = (1.0 - alpha_) * ema_ + alpha_ * score;

    ring_[ring_i_] = score;
    ring_i_ = (ring_i_ + 1) % 3;
    const double rollmax = std::max({ ring_[0], ring_[1], ring_[2] });

    const double s = std::max(score, rollmax);
    const double d = s - ema_;
    const bool trip = (d > trig_abs_) || (s > trig_rel_ * std::max(1.0, ema_));
    const bool cooldown_ok = (now_s - last_trigger_time_) >= cooldown_s_;
    bool fired = false;

    if (!in_high_) {
        if (trip && cooldown_ok) {
            in_high_ = true;
            fired = true;
            last_trigger_time_ = now_s;
            // Prevent baseline from instantly swallowing spike.
            ema_ = 0.9 * ema_ + 0.1 * s;
        }
    }
    else {
        if (delta < rel_abs_off_)
            in_high_ = false;
    }

    return fired;
}

void HitFlashDetector::Reset() {
    initialized_ = false;
    in_high_ = false;
    ema_ = 0.0;
    last_trigger_time_ = -1e9;
    ring_ = { 0, 0, 0 };
    ring_i_ = 0;
}
