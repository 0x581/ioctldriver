#pragma once

#include "driver.h"
#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>

static DWORD getPID(const wchar_t* processName)
{
    DWORD pid = 0;

    PROCESSENTRY32 processEntry{};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    const auto processSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (processSnapShot == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        std::cout << "Process snapshot failed: " << error << std::endl;

        return 0;
    }

    while (Process32Next(processSnapShot, &processEntry))
    {
        if (wcscmp(processName, processEntry.szExeFile) == 0)
        {
            pid = processEntry.th32ProcessID;
            break;
        }
    }

    if (processSnapShot)
        CloseHandle(processSnapShot);

    return pid;
}

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
{
    std::uintptr_t module_base = 0;


    HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap_shot == INVALID_HANDLE_VALUE)
        return module_base;

    MODULEENTRY32W entry = {};

    entry.dwSize = sizeof(decltype(entry));
    entry.dwSize = sizeof(decltype(entry));

    if (Module32FirstW(snap_shot, &entry) == TRUE) {
        if (wcsstr(module_name, entry.szModule) != nullptr) {
            module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
        }
        else {
            while (Module32NextW(snap_shot, &entry) == TRUE) {
                if (wcsstr(module_name, entry.szModule) != nullptr) {
                    module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                    break;
                }
            }
        }
    }

    CloseHandle(snap_shot);

    return module_base;
}


int main()
{
    const DWORD pid = getPID(L"Notepad.exe");

    if (pid == 0) {
        std::cout << "Failed to find notepad.\n";
        std::cin.get();
        return 1;
    }


    const HANDLE driver = CreateFile(L"\\\\.\\AssPileDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (driver == INVALID_HANDLE_VALUE) {
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