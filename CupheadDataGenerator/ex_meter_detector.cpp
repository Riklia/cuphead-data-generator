#include "ex_meter_detector.h"

#include "constants.h"

namespace {

cv::Rect ClampRect(const cv::Rect& rect, const cv::Size& s) {
    const cv::Rect clumped_rect = rect & cv::Rect(0, 0, s.width, s.height);
    return (clumped_rect.width > 0 && clumped_rect.height > 0) ? clumped_rect : cv::Rect();
}

}  // namespace


int ExMeterDetector::FindExValue(const cv::Mat& frame_bgr, cv::Mat* visual_dbg) {
    if (frame_bgr.empty() || ex_rect_.empty()) {
        return 0;
    }

    cv::Mat roi = frame_bgr(ex_rect_);
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> ch;
    cv::split(hsv, ch);
    cv::Mat v = ch[2];

    if (p_.blur_ksize >= 3 && (p_.blur_ksize % 2) == 1) {
        cv::medianBlur(v, v, p_.blur_ksize);
    }

    cv::Mat mask;
    cv::threshold(v, mask, p_.threshold, 255, cv::THRESH_BINARY);

    cv::Mat k = cv::getStructuringElement(cv::MORPH_RECT, { 3,3 });
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, k);

    const float slot_w = ex_rect_.width / (constants::max_player_ex + (constants::max_player_ex - 1) * p_.slot_gap);
    const float gap_w = slot_w * p_.slot_gap;

    float x = 0.f;
    for (int i = 0; i < constants::max_player_ex; ++i) {
        int sx = static_cast<int>(std::round(x));
        int sw = static_cast<int>(std::round(slot_w));
        cv::Rect slot_r(sx, 0, sw, ex_rect_.height);
        slot_r = ClampRect(slot_r, roi.size());

        if (!slot_r.empty()) {
            cv::Mat slot = mask(slot_r);
            const int white = cv::countNonZero(slot);
            const int area = slot_r.area();
            const float ratio = area > 0 ? static_cast<float>(white) / static_cast<float>(area) : 0.f;

            const bool raw_filled = (ratio >= p_.min_fill_ratio && ratio <= p_.max_fill_ratio);

            auto& [is_filled, on_cnt, off_cnt] = slot_states_[i];
            if (raw_filled) {
                on_cnt += 1;
                off_cnt = 0;
                if (!is_filled && on_cnt >= p_.frames_on_need) {
                    is_filled = true;
                }
            }
            else {
                off_cnt += 1;
                on_cnt = 0;
                if (is_filled && off_cnt >= p_.frames_off_need) {
                    is_filled = false;
                }
            }

            if (visual_dbg) {
                cv::Rect draw_r = slot_r + cv::Point(ex_rect_.x, ex_rect_.y);
                cv::Scalar col = is_filled ? constants::green_scalar : constants::red_scalar;
                cv::rectangle(*visual_dbg, draw_r, col, 2, cv::LINE_AA);
            }
        }

        x += slot_w + gap_w;
    }

    int value = 0;
    for (int i = static_cast<int>(slot_states_.size()) - 1; i >= 0; --i) {
        if (slot_states_[i].filled) {
	        value = i + 1;
        	break;
        }
    }
    return std::clamp(value, 0, constants::max_player_ex);
}
