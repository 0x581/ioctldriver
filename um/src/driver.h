#pragma once

#include <Windows.h>
#include "../../driver/src/driver.h"


namespace driver {
	bool do_nothing(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, codes::nothing, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
		const uintptr_t process_id = (uintptr_t)pid;

		Request r{ 0 };
		r.process_id = reinterpret_cast<HANDLE>(process_id);

		return DeviceIoControl(driver_handle, codes::attach_pid, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool attach_to_process(HANDLE driver_handle, const PCWCHAR process_name) {
		Request r{ 0 };
		r.buffer = (PVOID)process_name;

		return DeviceIoControl(driver_handle, codes::attach_name, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool unattach_from_process(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, codes::unattach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool process_is_alive(HANDLE driver_handle) {
		Request r{ 0 };
		const BOOL status = DeviceIoControl(driver_handle, codes::target_is_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	template <class T>
	T read_memory(HANDLE driver_handle, const uintptr_t addr) {
		T temp = {};

		Request r{ 0 };
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;
	}

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size) {
		Request r{ 0 };
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = buffer;
		r.size = size;
		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_size;
	}

	template <class T>
	void write_memory(HANDLE driver_handle, const uintptr_t addr, const T& value) {
		Request r{ 0 };
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	uintptr_t get_module(HANDLE driver_handle, const wchar_t* module_name) {
		Request r{ 0 };
		r.buffer = (PVOID)module_name;
		r.size = wcsnlen(module_name, MAX_PATH);

		if (r.size >= MAX_PATH)
			return 0;

		DeviceIoControl(driver_handle, codes::get_module, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	size_t set_debug_flags(HANDLE driver_handle, size_t new_flags) {
		Request r{ 0 };
		r.buffer = &new_flags;
		DeviceIoControl(driver_handle, codes::set_debug_flags, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_value;
	}
}