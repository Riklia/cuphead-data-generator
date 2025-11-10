#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

namespace debug_utils {

inline void print_coordinates_callback(int event, int x, int y, int, void* img_ptr) {
	if (event == cv::EVENT_LBUTTONDOWN) {
		std::cout << "\nClicked at (x=" << x << ", y=" << y << ")\n";
	}
}

}  // namespace debug_utils
