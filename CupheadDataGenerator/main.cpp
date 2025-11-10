#include <opencv2/opencv.hpp>
#include <iostream>

#include "cuphead_data_generator.h"
#include "win32_helpers.h"

int main() {
    //constexpr std::wstring_view process_name = L"DarkSoulsRemastered.exe";
    constexpr std::wstring_view process_name = L"Cuphead.exe";
    const DWORD pid = helpers::GetProcessIdByName(process_name);
    const std::vector<HWND> handles = helpers::GetAllWindowsFromProcessID(pid);
    if (handles.empty()) {
        std::wcout << "Cannot find a window for " << process_name;
        exit(1);
    }
    if (handles.size() > 1) {
        std::wcout << "Too many visible windows for " << process_name;
        exit(1);
    }
    WindowCapture w(handles.front());
    w.FocusWindow();
    CupheadDataGenerator data_generator(handles.front());
    //data_generator.StreamData();

    data_generator.PreviewStreamData();

    return 0;
}