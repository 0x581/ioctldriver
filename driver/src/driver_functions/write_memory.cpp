#pragma once

#include "driver_functions.h"
#include <kdebug.h>

extern "C" {
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
}


NTSTATUS write_memory(Process& process, RequestWriteMemory* request) {
	PVOID destination = request->destination;
	PVOID source = request->source;
	SIZE_T size = request->size;
	PSIZE_T return_size = &request->return_size;

	if (DEBUG_WRITE)
		debug("[+] Writing %d bytes from %p to %p.\n", size, source, destination);

	if (!process.Attached())
	{
		if (DEBUG_READ)
			debug("[-] Not attached to a procss.\n");

		return STATUS_INVALID_PARAMETER;
	}

	const auto status = MmCopyVirtualMemory(PsGetCurrentProcess(), source, process.eprocess,
		destination, size, KernelMode, return_size);

	if (DEBUG_WRITE) if (!NT_SUCCESS(status))
		debug("[-] Failed to write memory.\n");

	return status;
}
