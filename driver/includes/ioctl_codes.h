#pragma once

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)


#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)

#define DRIVER_FILE_DEVICE FILE_DEVICE_UNKNOWN

namespace ioctl_codes {
	//
	// Driver state
	constexpr unsigned long verify_connection =
		CTL_CODE(DRIVER_FILE_DEVICE, 0, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long attach =
		CTL_CODE(DRIVER_FILE_DEVICE, 10, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long unattach =
		CTL_CODE(DRIVER_FILE_DEVICE, 11, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// Target state
	constexpr unsigned long is_target_alive =
		CTL_CODE(DRIVER_FILE_DEVICE, 20, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// IO
	constexpr unsigned long read_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 100, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long write_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// Memory
	constexpr unsigned long allocate_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 200, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long free_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 201, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long protect_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 202, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long query_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 203, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long scan_memory =
		CTL_CODE(DRIVER_FILE_DEVICE, 204, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// Threads
	constexpr unsigned long create_thread =
		CTL_CODE(DRIVER_FILE_DEVICE, 300, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long remove_thread =
		CTL_CODE(DRIVER_FILE_DEVICE, 301, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// Modules
	constexpr unsigned long get_module_base =
		CTL_CODE(DRIVER_FILE_DEVICE, 400, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	constexpr unsigned long enumerate_modules =
		CTL_CODE(DRIVER_FILE_DEVICE, 401, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	//
	// Debugging
	constexpr unsigned long set_debug_flags =
		CTL_CODE(DRIVER_FILE_DEVICE, 500, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
}
