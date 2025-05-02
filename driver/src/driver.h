#pragma once

#include "debug.h"

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

		constexpr unsigned long set_debug_level =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 500, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	struct Request {
		void* process_id;

		void* target_address;
		void* buffer;

		size_t return_value;

		size_t size;
		size_t return_size;
	};
}
