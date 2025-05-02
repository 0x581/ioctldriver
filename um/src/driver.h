#pragma once

#include <Windows.h>

namespace driver {
	namespace codes {
		constexpr ULONG attach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr ULONG unattach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr ULONG target_is_alive =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 11, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr ULONG read =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr ULONG write =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 102, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr ULONG get_module =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 201, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	struct Request {
		HANDLE process_id;

		PVOID target_address;
		PVOID buffer;

		SIZE_T return_value;

		SIZE_T size;
		SIZE_T return_size;
	};


	bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
		Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool unattach_from_process(HANDLE driver_handle) {
		return DeviceIoControl(driver_handle, codes::unattach, nullptr, 0, nullptr, 0, nullptr, nullptr);
	}


	bool process_is_alive(HANDLE driver_handle, const DWORD pid) {
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
}