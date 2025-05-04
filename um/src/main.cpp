#pragma once

#include "driver.h"
#include "get_process.h"

#include <iostream>

#include <Windows.h>
#include <chrono>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;

void run_function(const HANDLE driver, const size_t pid, const size_t iterations) {

	const auto start_time = Time::now();

	const auto module_addr = get_module_base(pid, L"Notepad.exe");

	const auto percentage_steps = iterations / 100;

	auto i = 1;

	struct Data {
		char data[4096];
	};

	char buffer[sizeof(Data)] = { 0 };

	for (; i <= iterations; ++i)
	{
		//const auto alive = driver::do_nothing(driver);
		driver::read_memory<size_t>(driver, module_addr + 0x10);

		if (i % (percentage_steps * 5) == 0)
		{
			const int percentage = i / percentage_steps;
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


	const auto old_debug_level = driver::set_debug_level(driver, DEFAULT_FLAGS);

	const auto pid = driver::attach_to_process(driver, L"Notepad.exe");

	if (pid)
	{
		std::cout << "Attachment successful.\n";
		run_function(driver, pid, 10000000);
	}

	const auto unattach_success = driver::unattach_from_process(driver);

	if (unattach_success)
		std::cout << "Unattached from process.\n";
	else
		std::cout << "Failed to unattach from process.\n";

	CloseHandle(driver);
	std::cin.get();
	return 0;
}