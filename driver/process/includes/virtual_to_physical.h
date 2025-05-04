#pragma once

#include <ntifs.h>

#include "process_struct.h"

NTSTATUS ReadProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read = nullptr);

NTSTATUS WriteProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written);

template <class T>
T ReadProcessMemory(Process& process, uintptr_t addr) {
	T temp = {};

	ReadProcessMemory(process, (PVOID)addr, &temp, sizeof(T), nullptr);

	return temp;
}

template <class T>
T ReadProcessMemory(Process& process, PVOID addr) {
	T temp = {};

	ReadProcessMemory(process, addr, &temp, sizeof(T), nullptr);

	return temp;
}
