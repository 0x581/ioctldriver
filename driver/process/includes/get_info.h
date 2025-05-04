#pragma once

#include <ntifs.h>

HANDLE getProcessId(PEPROCESS pProcess);

uintptr_t getProcessCr3(PEPROCESS pProcess);

uintptr_t getKernelDirBase();

uintptr_t getProcessBaseAddress(PEPROCESS pProcess);

PLONG getProcessExitStatusAddress(PEPROCESS pProcess);
