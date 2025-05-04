#pragma once

#include <Windows.h>

union DebugFlags {
	size_t raw;

	struct {
		size_t device_control : 1;
		//
		// Initialization/cleanup
		size_t attach : 1;
		size_t unattach : 1;
		size_t process_struct : 1;
		size_t target_alive : 1;
		//
		// Virtual memory operations
		size_t read : 1;
		size_t write : 1;
		size_t linear_translation : 1;
		//
		// Walking process structures
		size_t get_process : 1;
		size_t get_module : 1;
		//
		// Debug Flags
		size_t set_debug_level : 1;

		// Reserved for future use
		size_t reserved : 53;
	} flags;
};


constexpr DebugFlags DEFAULT_FLAGS = {
	.flags {
			  .device_control = 0,
					  .attach = 1,
					.unattach = 1,
			  .process_struct = 1,
				.target_alive = 1,
						.read = 0,
					   .write = 0,
		  .linear_translation = 0,
				 .get_process = 0,
				  .get_module = 0,
			 .set_debug_level = 1,
	}
};

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)

namespace driver {
	namespace codes {
		constexpr unsigned long nothing =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long attach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long unattach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long target_is_alive =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 11, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long read =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long write =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 102, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long get_module =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 201, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		constexpr unsigned long set_debug_flags =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 500, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	struct Request {
		void* pid;

		void* address;
		void* buffer;

		size_t value;
		size_t return_value;

		size_t size;
		size_t return_size;
	};

	bool driver::do_nothing(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, codes::nothing, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}


	bool attach_to_process(HANDLE driver_handle, const PCWCHAR process_name) {
		Request r{ 0 };
		r.buffer = (PVOID)process_name;

		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool unattach_from_process(HANDLE driver_handle) {
		Request r{ 0 };
		return DeviceIoControl(driver_handle, codes::unattach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	bool process_is_alive(HANDLE driver_handle) {
		Request r{ 0 };
		const BOOL status = DeviceIoControl(driver_handle, codes::target_is_alive, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return status && r.return_value;
	}

	uintptr_t get_module_base(HANDLE driver_handle, const wchar_t* module_name) {
		Request r{ 0 };
		r.buffer = (PVOID)module_name;
		r.size = wcsnlen(module_name, MAX_PATH);

		if (r.size >= MAX_PATH)
			return 0;

		DeviceIoControl(driver_handle, codes::get_module, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	size_t set_debug_flags(HANDLE driver_handle, DebugFlags new_flags) {
		Request r{ 0 };
		r.value = new_flags.raw;

		DeviceIoControl(driver_handle, codes::set_debug_flags, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return r.return_value;
	}

	size_t read_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size) {
		Request r{ 0 };
		r.address = reinterpret_cast<PVOID>(addr);
		r.buffer = buffer;
		r.size = size;
		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		return r.return_size;
	}

	bool write_memory(HANDLE driver_handle, const uintptr_t addr, PVOID buffer, const size_t size) {
		Request r{ 0 };
		r.address = reinterpret_cast<PVOID>(addr);
		r.buffer = buffer;
		r.size = size;

		return DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

}
