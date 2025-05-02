#pragma once

#include <Windows.h>
#include "../../driver/src/driver.h"


namespace driver {
	bool do_nothing(HANDLE driver_handle) {
		Request r;
		return DeviceIoControl(driver_handle, codes::nothing, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
		Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool unattach_from_process(HANDLE driver_handle) {
		Request r;
		return DeviceIoControl(driver_handle, codes::unattach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool process_is_alive(HANDLE driver_handle) {
		Request r;
		const BOOL status = DeviceIoControl(driver_handle, codes::target_is_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	template <class T>
	T read_memory(HANDLE driver_handle, const uintptr_t addr) {
		T temp = {};

		Request r;
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;
	}

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size) {
		Request r;
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = buffer;
		r.size = size;
		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_size;
	}

	template <class T>
	void write_memory(HANDLE driver_handle, const uintptr_t addr, const T& value) {
		Request r;
		r.target_address = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	uintptr_t get_module(HANDLE driver_handle, const PWCHAR module_name) {
		Request r;
		r.buffer = (PVOID)module_name;
		r.size = wcsnlen(module_name, MAX_PATH + 4);

		if (r.size >= MAX_PATH + 4)
			return 0;

		DeviceIoControl(driver_handle, codes::get_module, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	size_t set_debug_level(HANDLE driver_handle, size_t new_flags) {
		Request r;
		r.buffer = &new_flags;
		DeviceIoControl(driver_handle, codes::set_debug_level, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_value;
	}
}