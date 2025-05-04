#pragma once

#include <Windows.h>
#include "../src/driver.h"


namespace driver {
	bool do_nothing(HANDLE driver_handle);

	bool attach_to_process(HANDLE driver_handle, const PCWCHAR process_name);

	bool unattach_from_process(HANDLE driver_handle);

	bool process_is_alive(HANDLE driver_handle);

	uintptr_t get_module_base(HANDLE driver_handle, const wchar_t* module_name);

	size_t set_debug_flags(HANDLE driver_handle, DebugFlags new_flags);

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size);

	bool write_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size);
}
