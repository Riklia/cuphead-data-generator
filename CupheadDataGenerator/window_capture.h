#pragma once

#include <windows.h>
#include <opencv2/opencv.hpp>

#include "frame_capture_worker.h"

// Provides data (capture) from the given window by window handle. 
class WindowCapture : public FrameSource {
public:
    explicit WindowCapture(const HWND window_handle) : window_handle_(window_handle) {}
    const HWND& GetWindowHandle() const { return window_handle_; }
    void SetWindowHandle(const HWND window_handle) { window_handle_ = window_handle; }
    // Returns true when a handle points to a window. 
    bool IsValid() const {
        return IsWindow(window_handle_);
    }
    // Restores window if it is minimized.
    void RestoreWindow() const {
        ShowWindow(window_handle_, SW_RESTORE);
    }
    // Focuses on window.
    void FocusWindow() const {
        ShowWindow(window_handle_, SW_MINIMIZE);
        RestoreWindow();
    }
    // Captures and returns the window context.
    cv::Mat Capture() const override;

private:
    HWND window_handle_;
};
