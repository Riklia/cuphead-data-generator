#include "window_capture.h"

cv::Mat WindowCapture::Capture() const {
	if (!IsValid()) {
		std::cout << "Invalid window handle (HWND).\n";
		return {};
	}
	const HDC window_device_context = GetDC(window_handle_);
	const HDC window_compatible_dc = CreateCompatibleDC(window_device_context);

	RECT window_rect;
	GetWindowRect(window_handle_, &window_rect);
	const int width = window_rect.right - window_rect.left;
	const int height = window_rect.bottom - window_rect.top;
	if (width <= 0 || height <= 0) {
		std::cout << "Invalid window dimensions.\n";
		DeleteDC(window_compatible_dc);
		ReleaseDC(window_handle_, window_device_context);
		return {};
	}

	const HBITMAP h_bitmap = CreateCompatibleBitmap(window_device_context, width, height);
	SelectObject(window_compatible_dc, h_bitmap);
	// Transfer data to `window_compatible_dc`.
	// Note that if we want to capture game window (that probably uses DirectX),
	// BitBlt is not suitable for this purpose.
	if (!PrintWindow(window_handle_, window_compatible_dc, PW_RENDERFULLCONTENT)) {
		std::cout << "PrintWindow failed.\n";
	}

	BITMAPINFOHEADER bitmap_info_header;
	bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bitmap_info_header.biWidth = width;
	// `biHeight` is negative to ensure top-down bitmap.
	bitmap_info_header.biHeight = -height;
	bitmap_info_header.biPlanes = 1;
	// `biBitCount` must be 32 to capture all 3 channels.
	// https://learn.microsoft.com/en-us/windows/win32/wmdm/-bitmapinfoheader
	bitmap_info_header.biBitCount = 32;
	bitmap_info_header.biCompression = BI_RGB;
	bitmap_info_header.biSizeImage = 0;
	bitmap_info_header.biXPelsPerMeter = 0;
	bitmap_info_header.biYPelsPerMeter = 0;
	bitmap_info_header.biClrUsed = 0;
	bitmap_info_header.biClrImportant = 0;

	cv::Mat image_mat(height, width, CV_8UC4);
	// Transfer the data from `bitmap_info_header` to `image_mat`.
	GetDIBits(window_compatible_dc, h_bitmap, 0, height, image_mat.data,
	          reinterpret_cast<BITMAPINFO*>(&bitmap_info_header), DIB_RGB_COLORS);

	// Memory allocated for GDI objects must be cleaned manually, otherwise there is a risk of
	// running out of memory or exceed the limit of GDI objects per process.
	DeleteObject(h_bitmap);
	DeleteDC(window_compatible_dc);
	ReleaseDC(window_handle_, window_device_context);

	cv::Mat image_mat_bgr;
	cv::cvtColor(image_mat, image_mat_bgr, cv::COLOR_BGRA2BGR);

	return image_mat_bgr;
}
