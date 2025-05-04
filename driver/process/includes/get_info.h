#pragma once

#include <ntifs.h>

HANDLE processGetPid(PEPROCESS pProcess);

uintptr_t processGetCr3(PEPROCESS pProcess);

uintptr_t getKernelDirBase();

uintptr_t processGetBaseAddress(PEPROCESS pProcess);

PPEB processGetPebPtr(PEPROCESS pProcess);

PLONG processGetExitStatusAddress(PEPROCESS pProcess);
