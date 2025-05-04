#pragma once

#include "../includes/get_info.h"

#include "../includes/offsets.h"

#include "kdebug.h"

#define CR3_MASK ~(PAGE_SIZE - 1);

HANDLE getProcessId(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	HANDLE pid = *(PHANDLE)(process + getProcessIdOffset());

	return pid;
}

//check normal dirbase if 0 then get from UserDirectoryTableBase
uintptr_t getProcessCr3(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t kernel_dirbase = *(uintptr_t*)(process + KernelDirectoryTableBaseOffset) & CR3_MASK;

	if (DEBUG_PROCESS_STRUCT)
		debug("[+] Getting process kernel CR3: 0x%p\n", kernel_dirbase);

	if (kernel_dirbase != 0)
		return kernel_dirbase;

	uintptr_t user_dirbase = *(uintptr_t*)(process + getProcessUserDirectoryTableBaseOffset()) & CR3_MASK;

	if (DEBUG_PROCESS_STRUCT)
		debug("[~] Getting process user CR3: 0x%p\n", user_dirbase);

	return user_dirbase;

}

uintptr_t getKernelDirBase() {
	uintptr_t process = (uintptr_t)PsGetCurrentProcess();

	uintptr_t cr3 = *(uintptr_t*)(process + KernelDirectoryTableBaseOffset);

	return cr3;
}

uintptr_t getProcessBaseAddress(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t Base = *(uintptr_t*)(process + getProcessSectionBaseAddressOffset());

	return Base;
}

uintptr_t getProcessPebPtr(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	uintptr_t pPeb = *(uintptr_t*)(process + getProcessPebPtrOffset());

	return pPeb;
}

PLONG getProcessExitStatusAddress(PEPROCESS pProcess) {
	uintptr_t process = (uintptr_t)pProcess;

	PLONG exit_status = (PLONG)(process + getProcessExitStatusOffset());

	return exit_status;
}
