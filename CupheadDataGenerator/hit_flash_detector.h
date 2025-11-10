#pragma once

#include <array>
#include <opencv2/opencv.hpp>

/**
 * HitFlashDetector detects short brightness spikes in a boss ROI. Act like a proxy
 * for hit events.
 *
 * Ech frame, the detector measures mean brightness of the central crop of the ROI,
 * keeps an exponential moving average (EMA) baseline, and reports a hit when brightness
 * rises sharply above that baseline (absolute or relative threshold).
 *
 * Typical tuning:
 *  alpha: 0.02-0.05, trig_abs: 6-10, trig_rel: 1.02-1.05.
 */ 


class HitFlashDetector {
public:
    explicit HitFlashDetector(double alpha = 0.03,
        double trig_abs = 8.0,
        double trig_rel = 1.04,
        double rel_abs_off = 3.0,
        double cooldown_s = 0.15,
        double center_crop = 0.7)
        : alpha_(alpha),
        trig_abs_(trig_abs),
        trig_rel_(trig_rel),
        rel_abs_off_(rel_abs_off),
        cooldown_s_(cooldown_s),
        center_crop_(center_crop) {
    }

    // Returns true if a flash (hit) was detected in this frame.
    bool Update(const cv::Mat& boss_bgr);

    // Resets internal state.
    void Reset();

private:
    double alpha_;
    double trig_abs_;
    double trig_rel_;
    double rel_abs_off_;
    double cooldown_s_;
    double center_crop_;

    bool initialized_ = false;
    bool in_high_ = false;
    double ema_ = 0.0;
    double last_trigger_time_ = -1e9;
    std::array<double, 3> ring_{ {0,0,0} };
    int ring_i_ = 0;

    double ComputeBrightnessScore(const cv::Mat& bgr) const;
};
