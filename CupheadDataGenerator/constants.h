#pragma once

#include <opencv2/opencv.hpp>

namespace constants {

static constexpr int esc_key = 27;

static const cv::Scalar red_scalar(0, 0, 255);
static const cv::Scalar green_scalar(0, 255, 0);
static const cv::Scalar blue_scalar(255, 0, 0);
static const cv::Scalar yellow_scalar(0, 255, 255);
static const cv::Scalar orange_scalar(0, 128, 255);
static const cv::Scalar black_scalar(0, 0, 0);
static const cv::Scalar pink_scalar(255, 100, 255);
static const cv::Scalar grey_scalar(128, 128, 128);

static constexpr int max_player_hp = 3;
static constexpr int max_player_ex = 5;

static const cv::Rect hp_badge( 25, 700, 75, 30);
static const cv::Rect ex_badge( 110, 700, 90, 30);

static constexpr int fps_window = 30;

}  // namespace constants
