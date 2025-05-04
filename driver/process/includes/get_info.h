#pragma once

#include <ntifs.h>

uintptr_t GetProcessCr3(PEPROCESS pProcess);

uintptr_t GetKernelDirBase();

uintptr_t GetProcessBaseAddress(PEPROCESS pProcess);
