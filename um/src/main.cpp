#pragma once

#include "driver.h"
#include "get_process.h"

#include <iostream>

#include <Windows.h>
#include <chrono>

int main() {
	const DWORD pid = getPID(L"Notepad.exe");

	if (pid == 0)
	{
		std::cout << "Failed to find notepad.\n";
		std::cin.get();
		return 1;
	}


	const HANDLE driver = CreateFile(L"\\\\.\\AssPileDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create our driver handle.\n";
		std::cin.get();
		return 1;
	}


	const auto old_debug_level = driver::set_debug_level(driver, DEFAULT_FLAGS);

	if (driver::attach_to_process(driver, pid))
		std::cout << "Attachment successful.\n";

	const auto module_addr = get_module_base(pid, L"Notepad.exe");


	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;

	const auto start_time = Time::now();

	constexpr auto iterations = 10000000;
	constexpr auto percentage_steps = iterations / 100;

	auto i = 1;

	struct Data {
		char data[4096];
	};

	char buffer[sizeof(Data)] = { 0 };

	for (; i <= iterations; ++i)
	{
		const auto alive = driver::process_is_alive(driver);

		if (!alive)
		{
			std::cout << "Target process is no longer alive.\n";
			break;
		}

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

	const auto unattach_success = driver::unattach_from_process(driver);

	if (unattach_success)
		std::cout << "Unattached from process.\n";
	else
		std::cout << "Failed to unattach from process.\n";

	CloseHandle(driver);
	std::cin.get();
	return 0;
}