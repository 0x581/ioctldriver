#pragma once

#include "driver_functions.h"
#include <kdebug.h>


#ifndef USE_MANUAL_WRITE
extern "C" {
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
}
#endif


NTSTATUS read_memory(Process& process, RequestReadMemory* request) {
	PVOID destination = request->destination;
	PVOID source = request->source;
	SIZE_T size = request->size;
	PSIZE_T return_size = &request->return_size;

	if (DEBUG_READ)
		debug("[+] Reading %d bytes from %p to %p.\n", size, source, destination);

	if (!process.Attached())
	{
		if (DEBUG_READ)
			debug("[-] Not attached to a procss.\n");

		return STATUS_INVALID_PARAMETER;
	}

#ifdef USE_MANUAL_READ
	const auto status = ReadProcessMemory(process.cr3, source, destination, size, return_size);
#else
	const auto status = MmCopyVirtualMemory(process.eprocess, source, PsGetCurrentProcess(),
		destination, size, KernelMode, return_size);
#endif

	if (DEBUG_READ) if (!NT_SUCCESS(status))
		debug("[-] Failed to read memory.\n");

	return status;
}
