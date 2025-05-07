#pragma once

typedef unsigned long long uintptr_t;
typedef uintptr_t DriverFlags;

#include <Windows.h>

namespace apd {
	HANDLE open_driver();

	void close_driver(HANDLE driver_handle);

	bool verify_connection(HANDLE driver_handle);

	bool attach_to_process(HANDLE driver_handle, const wchar_t* process_name);

	bool unattach_from_process(HANDLE driver_handle);

	bool is_process_alive(HANDLE driver_handle);

	uintptr_t get_module_base(HANDLE driver_handle, const wchar_t* module_name);

	DriverFlags set_debug_flags(HANDLE driver_handle, const DriverFlags new_flags);

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size);

	bool write_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size);
}
