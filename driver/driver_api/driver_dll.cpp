#pragma once

#define MYDRIVER_EXPORTS

#include "driver_api.h"

#include <ioapiset.h>

#include "ioctl_codes.h"
#include "requests.h"

ASSPILEDRIVER_API bool do_nothing(void* driver_handle) {
	Request r{ 0 };
	return DeviceIoControl(driver_handle, ioctl_codes::verify_connection, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}

ASSPILEDRIVER_API bool attach_to_process(void* driver_handle, const wchar_t* process_name) {
	Request r{ 0 };
	r.buffer = (PVOID)process_name;

	return DeviceIoControl(driver_handle, ioctl_codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}

ASSPILEDRIVER_API bool unattach_from_process(void* driver_handle) {
	Request r{ 0 };
	return DeviceIoControl(driver_handle, ioctl_codes::unattach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}

ASSPILEDRIVER_API bool process_is_alive(void* driver_handle) {
	Request r{ 0 };
	const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::is_target_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

	return status && r.return_value;
}

ASSPILEDRIVER_API uintptr_t get_module_base(void* driver_handle, const wchar_t* module_name) {
	Request r{ 0 };
	r.buffer = (PVOID)module_name;
	r.size = wcsnlen(module_name, MAX_PATH);

	if (r.size >= MAX_PATH)
		return 0;

	DeviceIoControl(driver_handle, ioctl_codes::get_module_base, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

	return r.return_value;
}

ASSPILEDRIVER_API size_t set_debug_flags(void* driver_handle, DebugFlags new_flags) {
	Request r{ 0 };
	r.value = new_flags.raw;

	DeviceIoControl(driver_handle, ioctl_codes::set_debug_flags, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

	return r.return_value;
}

ASSPILEDRIVER_API size_t read_memory(void* driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
	Request r{ 0 };
	r.address = reinterpret_cast<void*>(addr);
	r.buffer = buffer;
	r.size = size;
	DeviceIoControl(driver_handle, ioctl_codes::read_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	return r.return_size;
}

ASSPILEDRIVER_API bool write_memory(void* driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
	Request r{ 0 };
	r.address = reinterpret_cast<void*>(addr);
	r.buffer = buffer;
	r.size = size;

	return DeviceIoControl(driver_handle, ioctl_codes::write_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}
