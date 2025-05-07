#pragma once

#include "asspiledriver.h"

#include "driver_defines.h"
#include "ioctl_codes.h"
#include "requests.h"

namespace apd {
	HANDLE open_driver() {
		void* driver_handle = CreateFile(
			driver_file_w,
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);

		return driver_handle;
	}

	void close_driver(HANDLE driver_handle) {
		if (driver_handle != INVALID_HANDLE_VALUE)
			CloseHandle(driver_handle);
	}

	bool verify_connection(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, ioctl_codes::verify_connection, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool attach_to_process(HANDLE driver_handle, const wchar_t* process_name) {
		Request r{ 0 };
		r.buffer = (PVOID)process_name;

		return DeviceIoControl(driver_handle, ioctl_codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool unattach_from_process(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, ioctl_codes::unattach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool is_process_alive(HANDLE driver_handle) {
		Request r{ 0 };
		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::is_target_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status && r.return_value;
	}

	uintptr_t get_module_base(HANDLE driver_handle, const wchar_t* module_name) {
		Request r{ 0 };
		r.buffer = (PVOID)module_name;
		r.size = wcsnlen(module_name, MAX_PATH);

		if (r.size >= MAX_PATH)
			return 0;

		DeviceIoControl(driver_handle, ioctl_codes::get_module_base, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	DriverFlags set_debug_flags(HANDLE driver_handle, const DriverFlags new_flags) {
		Request r{ 0 };
		r.value = new_flags;

		DeviceIoControl(driver_handle, ioctl_codes::set_debug_flags, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
		Request r{ 0 };
		r.address = reinterpret_cast<void*>(addr);
		r.buffer = buffer;
		r.size = size;
		DeviceIoControl(driver_handle, ioctl_codes::read_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_size;
	}

	bool write_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
		Request r{ 0 };
		r.address = reinterpret_cast<void*>(addr);
		r.buffer = buffer;
		r.size = size;

		return DeviceIoControl(driver_handle, ioctl_codes::write_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}
}
