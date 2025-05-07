#pragma once

#include <Windows.h>

#include "asspiledriver.h"

#include <iostream>
#include <chrono>
#include <string>   
#include <iomanip>  

#define CLEANUP_AND_EXIT(code, message) \
{ \
	std::wcout << message << std::endl; \
	apd::close_driver(driver_handle); \
	std::cin.get(); \
	return code; \
}

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

#define TIME_FUNCTION(function_call) \
do { \
    auto time_func_start_time_ = std::chrono::high_resolution_clock::now(); \
    \
    auto time_func_iterations = (function_call); \
    \
    auto time_func_end_time_ = std::chrono::high_resolution_clock::now(); \
    \
	std::chrono::duration<double> time_func_duration_s_ = time_func_end_time_ - time_func_start_time_; \
    \
	auto time_func_ops_ = time_func_iterations / time_func_duration_s_.count(); \
    \
	std::cout << "TIMED: [" << STRINGIFY(function_call) << "]" << std::endl; \
    std::cout << "  -> Took: " << std::fixed << std::setprecision(6) << time_func_duration_s_.count() << " s" \
			  << " (" << time_func_ops_ << "ops/s)" << std::endl; \
} while(0)



int test_read_memory(HANDLE driver_handle, uintptr_t address, size_t size, int iterations) {
	void* buffer = new char[size];

	for (int i = 0; i < iterations; ++i)
	{
		auto bytes_read = apd::read_memory(driver_handle, address, buffer, size);
		if (bytes_read == 0)
		{
			std::wcerr << L"Failed to read memory." << std::endl;
			delete[] buffer;
			return i + 1;
		}
	}

	delete[] buffer;

	return iterations;
}


int main() {
	auto connected = apd::verify_connection(0x0);
	if (connected)
	{
		std::wcerr << L"Driver should not be already connected." << std::endl;
		std::cin.get();
		return 1;
	}

	HANDLE driver_handle = apd::open_driver();
	if (driver_handle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::wcerr << L"Failed to open device. Error: " << error << std::endl;
		std::cin.get();
		return 1;
	}

	connected = apd::verify_connection(driver_handle);
	if (!connected)
		CLEANUP_AND_EXIT(1, L"Driver connection failed.");

	bool success = apd::attach_to_process(driver_handle, L"Notsnjfg lajbsgfölajepad.exe");
	if (success)
		CLEANUP_AND_EXIT(1, L"Attached to invalid process.");

	success = apd::attach_to_process(driver_handle, L"Notepad.exe");
	if (!success)
		CLEANUP_AND_EXIT(1, L"Failed to attach to process.");

	const uintptr_t module_base = apd::get_module_base(driver_handle, L"Notepad.exe");
	if (module_base == 0)
		CLEANUP_AND_EXIT(1, L"Failed to get module base.");

	const uintptr_t module_base_2 = apd::get_module_base(driver_handle, L"Ndfasaaotepad.exe");
	if (module_base_2 != 0)
		CLEANUP_AND_EXIT(1, L"Found invalid module.");

	auto is_alive = apd::is_process_alive(driver_handle);
	if (!is_alive)
		CLEANUP_AND_EXIT(1, L"Process is not alive.");

	auto old_flags = apd::set_debug_flags(driver_handle, 0x0);

	apd::verify_connection(driver_handle);



	std::wcout << L"Module base: " << std::hex << module_base << std::endl;

	const uintptr_t address = module_base + 0x123456;

	TIME_FUNCTION(test_read_memory(driver_handle, 0, 64, 1000000));

	success = apd::detach_from_process(driver_handle);

	if (!success)
		CLEANUP_AND_EXIT(1, L"Failed to detach from process.");


	CLEANUP_AND_EXIT(0, L"Driver operations completed successfully.");
}