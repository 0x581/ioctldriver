#pragma once

#include "../includes/offsets.h"

#include "kdebug.h"


//https://ntdiff.github.io/
constexpr auto WINDOWS_10_1803 = 17134;
constexpr auto WINDOWS_10_1809 = 17763;
constexpr auto WINDOWS_10_1903 = 18362;
constexpr auto WINDOWS_10_1909 = 18363;
constexpr auto WINDOWS_10_2004 = 19041;
constexpr auto WINDOWS_10_20H2 = 19569;
constexpr auto WINDOWS_10_22H2 = 19041;
constexpr auto WINDOWS_11_23H2 = 22621;
constexpr auto WINDOWS_11_24H2 = 26100;


RTL_OSVERSIONINFOW& getVersionInfo() {
	static RTL_OSVERSIONINFOW ver = { 0 };

	if (ver.dwBuildNumber == 0)
	{
		RtlGetVersion(&ver);

		if (ver.dwBuildNumber < WINDOWS_10_1803)
			debug("[!] Driver is bein ran on an unsupported system! (Build Number: %d)\n", ver.dwBuildNumber);
	}

	return ver;
}

ULONG getBuildNumber() {
	RTL_OSVERSIONINFOW ver = getVersionInfo();

	return ver.dwBuildNumber;
}

uintptr_t processGetIdOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_1809)
	{
		offset = 0x2e0;
	}
	else if (version_num <= WINDOWS_10_1909)
	{
		offset = 0x2e8;
	}
	else if (version_num <= WINDOWS_11_23H2)
	{
		offset = 0x440;
	}
	else
	{
		offset = 0x1d0;
	}

	return offset;
}

uintptr_t processGetActiveProcessLinksOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	offset = processGetIdOffset() + 0x8;

	return offset;
}

uintptr_t processGetUserDirectoryTableBaseOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_1809)
	{
		offset = 0x278;
	}
	else if (version_num <= WINDOWS_10_1909)
	{
		offset = 0x280;
	}
	else if (version_num <= WINDOWS_10_22H2)
	{
		offset = 0x388;
	}
	else
	{
		offset = 0x158;
	}

	return offset;
}

uintptr_t processGetSectionBaseAddressOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_22H2)
	{
		offset = 0x520;
	}
	else
	{
		offset = 0x2b0;
	}

	return offset;
}

uintptr_t processGetPebPtrOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_1809)
	{
		offset = 0x624;
	}
	else if (version_num <= WINDOWS_10_1909)
	{
		offset = 0x3f8;
	}
	else if (version_num <= WINDOWS_11_23H2)
	{
		offset = 0x550;
	}
	else
	{
		offset = 0x2e0;
	}

	return offset;
}

uintptr_t processGetExitStatusOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_1809)
	{
		offset = 0x624;
	}
	else if (version_num <= WINDOWS_10_1909)
	{
		offset = 0x654;
	}
	else if (version_num <= WINDOWS_11_23H2)
	{
		offset = 0x7d4;
	}
	else
	{
		offset = 0x554;
	}

	return offset;
}

uintptr_t processGetImageFilePtrOffset() {
	static uintptr_t offset = 0x0;

	if (offset != 0x0)
		return offset;

	const auto version_num = getBuildNumber();

	if (version_num <= WINDOWS_10_1909)
	{
		offset = 0x448;
	}
	else if (version_num <= WINDOWS_11_23H2)
	{
		offset = 0x5a0;
	}
	else
	{
		offset = 0x330;
	}

	return offset;
}
