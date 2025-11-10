#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string_view>

namespace helpers {

// Finds all visible HWNDs associated with a process id.
std::vector<HWND> GetAllWindowsFromProcessID(DWORD target_pid);

// Returns PID by the process name.
DWORD GetProcessIdByName(std::wstring_view name);

// Returns base address by PID.
uintptr_t GetModuleBaseAddress(DWORD pid, std::wstring_view module_name);
}
