#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class HpClassifier {
public:
    explicit HpClassifier(const std::string& onnx_path, int img_size = 32)
        : img_size_(img_size) {
        net_ = cv::dnn::readNetFromONNX(onnx_path);
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    std::optional<int> classify(const cv::Mat& roi_bgr, float conf_threshold = 0.70f) const {
        if (roi_bgr.empty()) {
            return std::nullopt;
        }

        cv::Mat gray; cv::cvtColor(roi_bgr, gray, cv::COLOR_BGR2GRAY);
        const int img_size = img_size_;
        const float scale = std::min(img_size / static_cast<float>(gray.rows), img_size / static_cast<float>(gray.cols));
        const int nh = std::max(1, static_cast<int>(std::round(gray.rows * scale)));
        const int nw = std::max(1, static_cast<int>(std::round(gray.cols * scale)));

        cv::Mat resized; cv::resize(gray, resized, cv::Size(nw, nh), 0, 0, cv::INTER_AREA);
        cv::Mat canvas = cv::Mat::zeros(img_size, img_size, CV_32F);
        const int top = (img_size - nh) / 2;
        const int left = (img_size - nw) / 2;
        cv::Mat dst_roi = canvas(cv::Rect(left, top, nw, nh));
        resized.convertTo(dst_roi, CV_32F, 1.0 / 255.0);

        std::vector<int> shape = { 1, img_size, img_size, 1 };
        cv::Mat blob(4, shape.data(), CV_32F);
        std::memcpy(blob.ptr<float>(), canvas.ptr<float>(), static_cast<size_t>(img_size) * img_size * sizeof(float));

        net_.setInput(blob);
        cv::Mat logits = net_.forward();

        cv::Mat logits32; logits.convertTo(logits32, CV_32F);

        double max_logit;
    	cv::Point max_id;
        cv::minMaxLoc(logits32, nullptr, &max_logit, nullptr, &max_id);

        cv::Mat exp_logits; cv::exp(logits32 - static_cast<float>(max_logit), exp_logits);
        float denominator = static_cast<float>(cv::sum(exp_logits)[0]);
        float conf = 1.0f / denominator;

        int prediction = max_id.x;
        if (conf < conf_threshold) {
            return std::nullopt;
        }
        return prediction;
    }

private:
    int img_size_;
    mutable cv::dnn::Net net_;
};