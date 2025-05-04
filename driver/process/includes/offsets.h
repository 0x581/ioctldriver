#pragma once

#include <ntifs.h>

constexpr auto DirectoryTableBaseOffset = 0x28;

RTL_OSVERSIONINFOW& getVersionInfo();

ULONG getBuildNumber();

uintptr_t getUserDirectoryTableBaseOffset();

uintptr_t getProcessSectionBaseAddressOffset();

uintptr_t getProcessExitStatusOffset();
