#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

class GameMemoryReader {
public:
	explicit GameMemoryReader(const std::wstring_view process_name)
        : process_name_(process_name) {
    }

    ~GameMemoryReader() {
        if (h_proc_) {
            CloseHandle(h_proc_);
        }
    }

    bool Attach();

    uintptr_t FollowPointerChain(uintptr_t base_ptr, const std::vector<uintptr_t>& offsets) const;

    template<typename T>
    bool ReadValue(const uintptr_t address, T& value) {
        return ReadProcessMemory(h_proc_, (LPCVOID)address, &value, sizeof(T), nullptr);
    }

    uintptr_t GetBaseModule() const { return base_module_; }

private:
    std::wstring process_name_;
    DWORD pid_ = 0;
    HANDLE h_proc_ = nullptr;
    uintptr_t base_module_ = 0;
};
