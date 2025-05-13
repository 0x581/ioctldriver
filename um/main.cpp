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
		std::wcerr << L"Failed to open device. Error: " << GetLastError() << std::endl;
		std::cin.get();
		return 1;
	}


	apd::set_debug_flags(driver_handle, 0xFFFFFFFFFFFFFFFF);

	size_t* value = new size_t;

	*value = 6969;

	bool success;

	success = apd::attach_to_process(driver_handle, L"um.exe");
	if (!success)
		CLEANUP_AND_EXIT(1, L"Failed to attach to process.");


	size_t new_value = 420;


	std::cout << "Old value: " << std::hex << *value << std::endl;

	const auto written = apd::write_memory(driver_handle, (uintptr_t)value, &new_value, sizeof(size_t));

	std::cout << "Wrote " << written << " bytes to address: " << std::hex << value << std::endl;
	std::cout << "New value: " << std::hex << *value << std::endl;



	success = apd::detach_from_process(driver_handle);

	if (!success)
		CLEANUP_AND_EXIT(1, L"Failed to detach from process.");


	CLEANUP_AND_EXIT(0, L"Driver operations completed successfully.");
}