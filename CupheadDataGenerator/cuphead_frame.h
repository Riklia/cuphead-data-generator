#pragma once

#include <opencv2/opencv.hpp>

#include "constants.h"

class CupheadFrame {
    using Frame = cv::Mat;
public:
    explicit CupheadFrame(cv::Mat frame) : frame_(std::move(frame)) {
        LoadDigitTemplates();
    }

    bool Empty() const { return frame_.empty(); }

    cv::Mat GrayScaleNormalized() const {
        cv::Mat output;
        cv::cvtColor(frame_, output, cv::COLOR_BGR2GRAY);
        output.convertTo(output, CV_32F, 1.0 / 255.0);
        return output;
    }

    cv::Rect FindHpBadge() const;

private:
    // Preprocess a small image to an edge map (shared for ROI & templates)
    static cv::Mat ToEdgeMap(const cv::Mat& bgr) {
        cv::Mat ycrcb, y, blur, clahe_y, edges;
        cv::cvtColor(bgr, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> ch; cv::split(ycrcb, ch);
        const cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(ch[0], clahe_y);
        cv::GaussianBlur(clahe_y, blur, cv::Size(3, 3), 0);
        cv::Canny(blur, edges, 40, 120);
        return edges;
    }

   void LoadDigitTemplates() {
        for (size_t i = 0; i < health_templates_.size(); ++i) {
            const cv::Mat img = cv::imread(constants::hp_templates_path + "/" + std::to_string(i) + ".png");
            if (img.empty()) {
                throw std::runtime_error("Template " + std::to_string(i) + " is empty.");
            }
            health_templates_[i] = ToEdgeMap(img);
        }
    }

    Frame frame_;
    std::array<cv::Mat, constants::max_player_hp> health_templates_;
};
