#pragma once

#include "../includes/get_info.h"

#include "../includes/offsets.h"

#include "kdebug.h"

#define CR3_MASK ~(PAGE_SIZE - 1);

uintptr_t getKernelDirBase()
{
	uintptr_t process = (uintptr_t)PsGetCurrentProcess();

	uintptr_t cr3 = *(uintptr_t*)(process + KernelDirectoryTableBaseOffset);

	return cr3;
}

HANDLE processGetPid(PEPROCESS pProcess)
{
	uintptr_t process = (uintptr_t)pProcess;

	HANDLE pid = *(PHANDLE)(process + processGetIdOffset());

	return pid;
}

//check normal dirbase if 0 then get from UserDirectoryTableBase
uintptr_t processGetCr3(PEPROCESS pProcess)
{
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t kernel_dirbase = *(uintptr_t*)(process + KernelDirectoryTableBaseOffset) & CR3_MASK;

	if (kernel_dirbase != 0)
		return kernel_dirbase;

	uintptr_t user_dirbase = *(uintptr_t*)(process + processGetUserDirectoryTableBaseOffset()) & CR3_MASK;

	return user_dirbase;

}

uintptr_t processGetBaseAddress(PEPROCESS pProcess)
{
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t Base = *(uintptr_t*)(process + processGetSectionBaseAddressOffset());

	return Base;
}

PPEB processGetPebPtr(PEPROCESS pProcess)
{
	uintptr_t process = (uintptr_t)pProcess;

	PPEB pPeb = *(PPEB*)(process + processGetPebPtrOffset());

	return pPeb;
}

PLONG processGetExitStatusAddress(PEPROCESS pProcess)
{
	uintptr_t process = (uintptr_t)pProcess;

	PLONG exit_status = (PLONG)(process + processGetExitStatusOffset());

	return exit_status;
}
