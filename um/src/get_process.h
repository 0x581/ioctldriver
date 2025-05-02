#pragma once

#include <iostream>

#include <Windows.h>
#include <Tlhelp32.h>

static DWORD getPID(const wchar_t* processName) {
	const HANDLE snap_shot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap_shot == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::cout << "Process snapshot failed: " << error << std::endl;

		return 0;
	}

	PROCESSENTRY32 entry{ .dwSize = sizeof(entry) };
	DWORD pid = 0;

	if (Process32First(snap_shot, &entry))
	{
		do
		{
			if (wcsncmp(processName, entry.szExeFile, MAX_PATH) == 0)
			{
				pid = entry.th32ProcessID;
				break;
			}
		} while (Process32Next(snap_shot, &entry));
	}

	if (snap_shot)
		CloseHandle(snap_shot);

	return pid;
}

static uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
	HANDLE snap_shot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::cout << "Module snapshot failed: " << error << std::endl;

		return 0;
	}

	MODULEENTRY32 entry = { .dwSize = sizeof(entry) };
	uintptr_t module_base = 0;

	if (Module32First(snap_shot, &entry))
	{
		do
		{
			if (wcsstr(module_name, entry.szModule) == 0)
			{
				module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
				break;
			}
		} while (Module32Next(snap_shot, &entry));
	}

	CloseHandle(snap_shot);

	return module_base;
}


static void list_modules(const DWORD pid) {
	HANDLE snap_shot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::cout << "Module snapshot failed: " << error << std::endl;

		return;
	}

	MODULEENTRY32 entry = { .dwSize = sizeof(entry) };

	if (Module32First(snap_shot, &entry))
	{
		do
		{
			std::wcout << L"Module: " << entry.szModule << L" Base Address: " << entry.modBaseAddr << std::endl;
		} while (Module32Next(snap_shot, &entry));
	}

	CloseHandle(snap_shot);
}