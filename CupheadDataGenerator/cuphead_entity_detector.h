#pragma once

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <vector>

enum class EntityType {
	player = 0,
	boss = 1,
	projectile = 2,
	parryable = 3,

	invalid = -1
};

inline const char* ToString(EntityType entity_type) {
    switch (entity_type) {
    case EntityType::player:
        return "PLAYER";
    case EntityType::boss:
        return "BOSS";
    case EntityType::projectile:
        return "PROJECTILE";
    case EntityType::parryable:
        return "PARRYABLE";
    case EntityType::invalid:
        return "INVALID";
    }
    return "UNKNOWN";
}


struct EntityDetection {
    cv::Rect box;
    EntityType type;
    float confidence;
};

class CupheadEntityDetector {
public:
    explicit CupheadEntityDetector(const std::wstring& onnx_path, int input_size);

    // Runs detection on a BGR frame. Returns vector of detected entities.
    std::vector<EntityDetection> DetectEntities(const cv::Mat& bgr_frame,
        float conf_threshold = 0.5f,
        float iou_threshold = 0.50f);

private:
    Ort::Env env_;
    Ort::SessionOptions session_opts_;
    Ort::Session session_{ nullptr };
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> allocator_;
    Ort::MemoryInfo meminfo_{ nullptr };
    std::string input_name_;
    std::string output_name_;
    int input_size_;
};
