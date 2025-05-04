#pragma once

#include <ntifs.h>

constexpr auto KernelDirectoryTableBaseOffset = 0x28;

RTL_OSVERSIONINFOW& getVersionInfo();

ULONG getBuildNumber();

uintptr_t getProcessIdOffset();

uintptr_t getProcessUserDirectoryTableBaseOffset();

uintptr_t getProcessSectionBaseAddressOffset();

uintptr_t getProcessPebPtrOffset();

uintptr_t getProcessExitStatusOffset();
