#pragma once

#include <opencv2/opencv.hpp>

#include "constants.h"

class ExMeterDetector {
public:
    struct SlotState {
	    bool filled = false;
    	int on_cnt = 0;
    	int off_cnt = 0;
    };

    struct Params {
        float slot_gap = 0.05f;
        int blur_ksize = 3;
        int threshold = 200;
        float min_fill_ratio = 0.3f;
        float max_fill_ratio = 0.5f;
        int frames_on_need = 3;
        int frames_off_need = 10;
    };

    explicit ExMeterDetector(cv::Rect ex_rect, const Params& p = Params{}) : ex_rect_(ex_rect), p_(p) {}

    int FindExValue(const cv::Mat& frame_bgr, cv::Mat* visual_dbg = nullptr);

private:
    cv::Rect ex_rect_;
    std::array<SlotState, constants::max_player_ex> slot_states_;

    Params p_;
};
