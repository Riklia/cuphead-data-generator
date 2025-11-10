#include "win32_helpers.h"

namespace helpers {

std::vector<HWND> GetAllWindowsFromProcessID(DWORD target_pid) {
	std::vector<HWND> output;
	HWND h_cur_wnd = nullptr;
	do {
		h_cur_wnd = FindWindowEx(nullptr, h_cur_wnd, nullptr, nullptr);
		DWORD current_pid = 0;
		GetWindowThreadProcessId(h_cur_wnd, &current_pid);
		if (current_pid == target_pid) {
			if (!IsWindowVisible(h_cur_wnd)) {
				continue;
			}
			output.push_back(h_cur_wnd);
		}
	}
	while (h_cur_wnd != nullptr);
	return output;
}

DWORD GetProcessIdByName(const std::wstring_view name) {
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32FirstW(snapshot, &entry)) {
		do {
			if (name == entry.szExeFile) {
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
		while (Process32NextW(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return 0;
}

uintptr_t GetModuleBaseAddress(const DWORD pid, const std::wstring_view module_name) {
	uintptr_t base_addr = 0;
	if (const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		snapshot != INVALID_HANDLE_VALUE) {
		MODULEENTRY32W module_entry;
		module_entry.dwSize = sizeof(module_entry);
		if (Module32FirstW(snapshot, &module_entry)) {
			do {
				if (module_name == module_entry.szModule) {
					base_addr = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
					break;
				}
			}
			while (Module32NextW(snapshot, &module_entry));
		}
		CloseHandle(snapshot);
	}
	return base_addr;
}

}
