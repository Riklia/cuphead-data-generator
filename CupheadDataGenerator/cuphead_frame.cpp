#include "cuphead_frame.h"

cv::Rect CupheadFrame::FindHpBadge() const {
	if (frame_.empty()) {
		return {};
	}
	return {25, 700, 75, 30};
}
