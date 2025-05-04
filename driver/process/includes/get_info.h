#pragma once

#include <ntifs.h>

uintptr_t getProcessCr3(PEPROCESS pProcess);

uintptr_t getKernelDirBase();

uintptr_t getProcessBaseAddress(PEPROCESS pProcess);

PLONG getProcessExitStatusAddress(PEPROCESS pProcess);
