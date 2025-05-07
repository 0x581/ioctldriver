#pragma once

#include <ntifs.h>

#include "process_struct.h"

NTSTATUS ReadProcessMemory(uintptr_t process_cr3, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read = nullptr);

NTSTATUS ReadProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read = nullptr);

NTSTATUS WriteProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written);

template <class T>
T ReadProcessMemory(uintptr_t cr3, PVOID addr) {
	T temp = {};

	ReadProcessMemory(cr3, addr, &temp, sizeof(T), nullptr);

	return temp;
}

template <class T>
T ReadProcessMemory(Process& process, uintptr_t addr) {
	T temp = {};

	ReadProcessMemory(process.cr3, (PVOID)addr, &temp, sizeof(T), nullptr);

	return temp;
}

template <class T>
T ReadProcessMemory(Process& process, PVOID addr) {
	T temp = {};

	ReadProcessMemory(process.cr3, addr, &temp, sizeof(T), nullptr);

	return temp;
}
