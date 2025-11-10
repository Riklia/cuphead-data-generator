#include "game_memory_reader.h"
#include "win32_helpers.h"

bool GameMemoryReader::Attach() {
    pid_ = helpers::GetProcessIdByName(process_name_);
    if (!pid_) {
        std::cout << "Process not found.\n";
        return false;
    }

    h_proc_ = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid_);
    if (!h_proc_) {
        std::cout << "Failed to open process.\n";
        return false;
    }

    base_module_ = helpers::GetModuleBaseAddress(pid_, process_name_);
    if (!base_module_) {
        std::cout << "Failed to get module base address.\n";
        return false;
    }

    std::cout << "Attached to process. Base module at: 0x"
        << std::hex << base_module_ << "\n" << std::dec;
    return true;
}

uintptr_t GameMemoryReader::FollowPointerChain(const uintptr_t base_ptr, const std::vector<uintptr_t>& offsets) const
{
	uintptr_t addr = base_module_ + base_ptr;
	for (size_t i = 0; i < offsets.size(); ++i) {
		if (!ReadProcessMemory(h_proc_, (LPCVOID)(addr), &addr, sizeof(addr), nullptr)) {
			std::cout << "Failed to read pointer at offset " << i << "\n";
			return 0;
		}
		addr += offsets[i];
	}
	return addr;
}




