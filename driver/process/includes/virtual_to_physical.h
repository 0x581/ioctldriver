#pragma once

#include <ntifs.h>

#include "process_struct.h"

NTSTATUS ReadProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read);

NTSTATUS WriteProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written);

template <class T>
T ReadProcessMemory(Process& process, uintptr_t addr) {
	T temp = {};
	size_t read = 0;

	ReadProcessMemory(process, (PVOID)addr, &temp, sizeof(T), &read);

	return temp;
}
