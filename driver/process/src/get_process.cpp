#pragma once

#include "../includes/get_process.h"

#include "../../kdebug/kdebug.h"

#include "../../windows_internals/ZwQuerySystemInformation.cpp"

#define POOL_TAG 'QdpA' // AssPileDriver Query


NTSTATUS findProcessIdByName(const PWCHAR process_name, HANDLE& pid) {
	NTSTATUS status;

	ULONG buffer_size = 0;
	PVOID buffer = NULL;


	PSYSTEM_PROCESS_INFORMATION processInfo;
	BOOLEAN found = FALSE;

	UNICODE_STRING unicode_name;
	RtlInitUnicodeString(&unicode_name, process_name);
	if (unicode_name.Buffer == nullptr)
	{
		if (DEBUG_ATTACH)
			debug("[-] Invalid process name specified\n");

		return STATUS_INVALID_PARAMETER;
	}


	status = ZwQuerySystemInformation(SystemProcessInformation, NULL, 0, &buffer_size);

	if (status != STATUS_INFO_LENGTH_MISMATCH || buffer_size == 0)
	{
		if (DEBUG_ATTACH)
			debug("[-] ZwQuerySystemInformation (size query) failed or returned zero size: 0x%X\n", status);

		return (status == STATUS_SUCCESS) ? STATUS_UNEXPECTED_IO_ERROR : status;
	}

	// Extra buffer to accound for newly opened processes
	buffer_size += 1024 * 10;

	buffer = ExAllocatePool2(POOL_FLAG_PAGED, buffer_size, POOL_TAG);

	if (buffer == nullptr)
	{
		if (DEBUG_ATTACH)
			debug("[-] ExAllocatePool2 failed to allocate %lu bytes\n", buffer_size);

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// --- Step 3: Query actual system process information ---
	status = ZwQuerySystemInformation(SystemProcessInformation, buffer, buffer_size, &buffer_size); // Can reuse bufferSize to get actual returned length if needed

	if (!NT_SUCCESS(status) || buffer_size == 0)
	{
		if (DEBUG_ATTACH)
			debug("[-] ZwQuerySystemInformation(data query) failed or returned zero size : 0x%X\n", status);

		ExFreePool(buffer);
		return status;
	}

	processInfo = (PSYSTEM_PROCESS_INFORMATION)buffer;

	while (true)
	{
		if (processInfo->ImageName.Buffer && processInfo->ImageName.Length > 0)
		{
			if (RtlCompareUnicodeString(&processInfo->ImageName, &unicode_name, TRUE) == 0)
			{
				found = TRUE;
				break;
			}
		}

		if (processInfo->NextEntryOffset == 0)
			break;

		ULONG_PTR nextOffset = (ULONG_PTR)processInfo + processInfo->NextEntryOffset;

		if (nextOffset >= (ULONG_PTR)buffer + buffer_size)
		{
			if (DEBUG_ATTACH)
				debug("[-] NextEntryOffset points outside buffer!\n");

			status = STATUS_INFO_LENGTH_MISMATCH;
			break;
		}

		processInfo = (PSYSTEM_PROCESS_INFORMATION)((char*)processInfo + processInfo->NextEntryOffset);
	}

	if (found)
	{
		pid = processInfo->UniqueProcessId;

		if (DEBUG_ATTACH)
			debug("[+] Found process %wZ (PID %d)\n", &processInfo->ImageName, pid);
	}
	else
	{
		pid = 0;

		if (DEBUG_ATTACH)
			debug("[-] Process %wZ not found\n", &unicode_name);
	}

	ExFreePool(buffer);

	if (!found)
		return STATUS_NOT_FOUND;

	return status;
}
