#pragma once

#include "driver.h"
#include "get_process.h"

#include <iostream>

#include <Windows.h>

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


	if (driver::attach_to_process(driver, pid))
		std::cout << "Attachment successful.\n";

	CloseHandle(driver);
	std::cin.get();
	return 0;
}