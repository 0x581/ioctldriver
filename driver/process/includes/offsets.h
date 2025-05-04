#pragma once

#include <ntifs.h>

constexpr auto KernelDirectoryTableBaseOffset = 0x28;

constexpr auto pebLdrOffset = 0x18;

constexpr auto ldrInLoadOrderModuleListOffset = 0x10;

RTL_OSVERSIONINFOW& getVersionInfo();

ULONG getBuildNumber();

uintptr_t processGetIdOffset();

uintptr_t processGetActiveProcessLinksOffset();

uintptr_t processGetUserDirectoryTableBaseOffset();

uintptr_t processGetSectionBaseAddressOffset();

uintptr_t processGetPebPtrOffset();

uintptr_t processGetExitStatusOffset();

uintptr_t processGetImageFilePtrOffset();
