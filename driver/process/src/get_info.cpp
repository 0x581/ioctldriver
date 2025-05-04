#pragma once

#include "../includes/get_info.h"

#include "../includes/offsets.h"


//check normal dirbase if 0 then get from UserDirectoryTableBase
uintptr_t getProcessCr3(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t process_dirbase = process + DirectoryTableBaseOffset;

	if (process_dirbase == 0)
	{
		uintptr_t process_userdirbase = *(uintptr_t*)(process + getUserDirectoryTableBaseOffset());
		return process_userdirbase;
	}

	return process_dirbase;
}

uintptr_t getKernelDirBase() {
	uintptr_t process = (uintptr_t)PsGetCurrentProcess();

	uintptr_t cr3 = *(uintptr_t*)(process + DirectoryTableBaseOffset);

	return cr3;
}

uintptr_t getProcessBaseAddress(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t Base = *(uintptr_t*)(process + getProcessSectionBaseAddressOffset());

	return Base;
}

PLONG getProcessExitStatusAddress(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	PLONG exit_status = (PLONG)(process + getProcessExitStatusOffset());

	return exit_status;
}
