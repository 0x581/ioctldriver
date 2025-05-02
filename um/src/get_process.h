#pragma once

#include <iostream>

#include <Windows.h>
#include <Tlhelp32.h>

static DWORD getPID(const wchar_t* processName) {
	DWORD pid = 0;

	PROCESSENTRY32 processEntry{};
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	const HANDLE processSnapShot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (processSnapShot == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::cout << "Process snapshot failed: " << error << std::endl;

		return 0;
	}

	while (Process32Next(processSnapShot, &processEntry))
	{
		if (wcsncmp(processName, processEntry.szExeFile, MAX_PATH) == 0)
		{
			pid = processEntry.th32ProcessID;
			break;
		}
	}

	if (processSnapShot)
		CloseHandle(processSnapShot);

	return pid;
}

static uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
	uintptr_t module_base = 0;


	HANDLE snap_shot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE)
		return module_base;

	MODULEENTRY32 entry = {};

	entry.dwSize = sizeof(decltype(entry));
	entry.dwSize = sizeof(decltype(entry));

	if (Module32First(snap_shot, &entry) == TRUE)
	{
		if (wcsstr(module_name, entry.szModule) != nullptr)
		{
			module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
		}
		else
		{
			while (Module32Next(snap_shot, &entry) == TRUE)
			{
				if (wcsstr(module_name, entry.szModule) != nullptr)
				{
					module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return module_base;
}
