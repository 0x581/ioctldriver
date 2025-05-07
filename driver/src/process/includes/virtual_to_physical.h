#pragma once

#include <ntifs.h>

#include "process_struct.h"

NTSTATUS ReadProcessMemory(uintptr_t process_cr3, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read = nullptr);

NTSTATUS ReadProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read = nullptr);

NTSTATUS WriteProcessMemory(uintptr_t process_cr3, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written = nullptr);

NTSTATUS WriteProcessMemory(Process& process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written = nullptr);
