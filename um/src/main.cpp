#pragma once

#include "driver.h"
#include "get_process.h"

#include <iostream>

#include <Windows.h>
#include <chrono>
#include <thread>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;

void test_read(const HANDLE driver, const DWORD pid, const size_t iterations) {

	const auto start_time = Time::now();

	const auto module_addr = get_module_base(pid, L"Notepad.exe");

	const auto percentage_steps = iterations / 100;

	char buffer[4096] = { 0 };

	auto i = 1;
	for (; i <= iterations; ++i)
	{
		driver::read_memory<size_t>(driver, module_addr + 0x10);

		if (iterations > 100)
			if (i % (percentage_steps * 5) == 0)
			{
				const auto percentage = i / percentage_steps;
				std::cout << percentage << "%" << std::endl;
			}
	}

	--i;

	const auto end_time = Time::now();

	const std::chrono::duration<float> elapsed_time = end_time - start_time;

	std::cout << "Elapsed time: " << elapsed_time.count() << " seconds\n";
	std::cout << (size_t)(i / elapsed_time.count()) << " ops/s (" << i << "ops / " << elapsed_time.count() << "s)\n";
}


void test_connection(const HANDLE driver, const DWORD pid, const size_t iterations) {

	const auto start_time = Time::now();

	const auto module_addr = get_module_base(pid, L"Notepad.exe");

	const auto percentage_steps = iterations / 100;

	auto i = 1;
	for (; i <= iterations; ++i)
	{
		driver::process_is_alive(driver);

		//std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (iterations > 100)
			if (i % (percentage_steps * 5) == 0)
			{
				const auto percentage = i / percentage_steps;
				std::cout << percentage << "%" << std::endl;
			}
	}

	--i;

	const auto end_time = Time::now();

	const std::chrono::duration<float> elapsed_time = end_time - start_time;

	std::cout << "Elapsed time: " << elapsed_time.count() << " seconds\n";
	std::cout << (size_t)(i / elapsed_time.count()) << " ops/s (" << i << "ops / " << elapsed_time.count() << "s)\n";
}





int main() {
	const HANDLE driver = CreateFile(L"\\\\.\\AssPileDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create our driver handle.\n";
		std::cin.get();
		return 1;
	}

	const auto old_debug_level = driver::set_debug_flags(driver, DEFAULT_FLAGS);

	const auto success = driver::attach_to_process(driver, L"Notepad.exe");

	if (success)
	{
		std::cout << "Attachment successful.\n";
		test_connection(driver, getPID(L"Notepad.exe"), 20);
	}
	else
		std::cout << "Failed to attach to process.\n";

	const auto unattach_success = driver::unattach_from_process(driver);

	if (unattach_success)
		std::cout << "Unattached from process.\n";
	else
		std::cout << "Failed to unattach from process.\n";

	CloseHandle(driver);
	std::cin.get();
	return 0;
}