#pragma once

#include <opencv2/opencv.hpp>

namespace constants {

static constexpr int esc_key = 27;

static const cv::Scalar yellow_scalar(0, 255, 255);
static const cv::Scalar green_scalar(0, 255, 0);
static const cv::Scalar orange_scalar(0, 128, 255);
static const cv::Scalar black_scalar(0, 0, 0);

static const std::string hp_templates_path = "templates/hp";

static constexpr int max_player_hp = 3;
static constexpr int max_cards = 5;

static constexpr int fps_window = 30;

}  // namespace constants
