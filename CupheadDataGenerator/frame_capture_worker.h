#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>

class FrameSource {
public:
    FrameSource() = default;
    virtual ~FrameSource() = default;

    FrameSource(const FrameSource&) = delete;
    FrameSource& operator=(const FrameSource&) = delete;
    FrameSource(FrameSource&&) = delete;
    FrameSource& operator=(FrameSource&&) = delete;

    virtual cv::Mat Capture() const = 0;
};

class FrameCaptureWorker {
public:
    explicit FrameCaptureWorker(FrameSource* src)
        : src_(src) {
    }

    FrameCaptureWorker(const FrameCaptureWorker&) = delete;
    FrameCaptureWorker& operator=(const FrameCaptureWorker&) = delete;
    FrameCaptureWorker(FrameCaptureWorker&&) = delete;
    FrameCaptureWorker& operator=(FrameCaptureWorker&&) = delete;

    ~FrameCaptureWorker() { Stop(); }

    void Start() {
        if (running_.exchange(true)) return;
        worker_ = std::thread([this] { Run(); });
    }

    void Stop() {
        if (!running_.exchange(false)) return;
        cv_ready_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    // Blocks until a frame is available or stopped; returns false if stopping.
    bool WaitAndGet(cv::Mat& out) {
        std::unique_lock lock(mtx_);
        cv_ready_.wait(lock, [&] { return !running_ || has_frame_; });
        if (!running_ && !has_frame_) return false;
        latest_.copyTo(out);
        has_frame_ = false;
        return true;
    }

    // Returns true if a frame was copied.
    bool TryGetLatest(cv::Mat& out) {
        std::lock_guard lock(mtx_);
        if (!has_frame_) return false;
        latest_.copyTo(out);
        has_frame_ = false;
        return true;
    }

private:
    void Run() {
        while (running_) {
            cv::Mat f = src_->Capture();
            if (f.empty()) continue;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                f.copyTo(latest_);
                has_frame_ = true;
            }
            cv_ready_.notify_one();
        }
    }

    FrameSource* src_;
    std::atomic<bool> running_{ false };
    std::thread worker_;

    std::mutex mtx_;
    std::condition_variable cv_ready_;
    cv::Mat latest_;
    bool has_frame_ = false;
};

