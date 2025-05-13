#pragma once

#include "asspiledriver.h"

#include "driver_defines.h"
#include "ioctl_codes.h"
#include "requests.h"

#include "Windows.h"

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
		RequestVerifyConnection r{ };
		return DeviceIoControl(driver_handle, ioctl_codes::verify_connection, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool attach_to_process(HANDLE driver_handle, const wchar_t* process_name, HANDLE* pid) {
		RequestAttach r{};
		r.pid = 0;
		wcsncpy_s(r.process_name, process_name, MAX_PATH + 1);

		const auto status = DeviceIoControl(driver_handle, ioctl_codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		if (pid != nullptr)
			*pid = r.pid;

		return status;
	}

	bool detach_from_process(HANDLE driver_handle) {
		RequestDetach r{};
		return DeviceIoControl(driver_handle, ioctl_codes::detach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool is_process_alive(HANDLE driver_handle) {
		RequestIsProcessAlive r{};
		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::is_process_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status && r.is_alive;
	}

	uintptr_t get_module_base(HANDLE driver_handle, const wchar_t* module_name) {
		if (wcsnlen(module_name, MAX_PATH + 1) > MAX_PATH)
			return 0;

		RequestGetModuleBase r{ };
		r.module_base = 0;
		wcsncpy_s(r.module_name, module_name, MAX_PATH + 1);

		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::get_module_base, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status ? r.module_base : -1;
	}

	DriverFlags set_debug_flags(HANDLE driver_handle, const DriverFlags new_flags) {
		RequestSetDebugFlags r{ };
		r.old_flags = 0;
		r.new_flags = new_flags;

		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::set_debug_flags, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status ? r.old_flags : -1;
	}

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
		RequestReadMemory r{ };
		r.source = reinterpret_cast<void*>(addr);
		r.destination = buffer;
		r.size = size;

		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::read_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status ? r.return_size : -1;
	}

	size_t write_memory(HANDLE driver_handle, const uintptr_t addr, void* buffer, const size_t size) {
		RequestWriteMemory r{ };
		r.destination = reinterpret_cast<void*>(addr);
		r.source = buffer;
		r.size = size;

		const BOOL status = DeviceIoControl(driver_handle, ioctl_codes::write_memory, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status ? r.return_size : -1;
	}
}
