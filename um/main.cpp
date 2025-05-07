#pragma once

#include <Windows.h>
#include <iostream>

#include "asspiledriver.h"

int main() {
	HANDLE driver_handle = apd::open_driver();
	if (driver_handle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::wcerr << L"Failed to open device. Error: " << error << std::endl;
		std::cin.get();
		return 1;
	}

	bool success = apd::attach_to_process(driver_handle, L"Notepad.exe");

	if (!success)
	{
		std::wcerr << L"Failed to attach to process." << std::endl;
		apd::close_driver(driver_handle);
		std::cin.get();
		return 1;
	}

	success = apd::unattach_from_process(driver_handle);

	if (!success)
	{
		std::wcerr << L"Failed to unattach from process." << std::endl;
		apd::close_driver(driver_handle);
		std::cin.get();
		return 1;
	}

	apd::close_driver(driver_handle);

	std::wcout << L"Driver operations completed successfully." << std::endl;

	std::cin.get();

	return 0;
}