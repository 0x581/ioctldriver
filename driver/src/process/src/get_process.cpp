#pragma once

#include "../includes/get_process.h"

#include "../includes/offsets.h"

#include "kdebug.h"

void GetBasePathUnicodeString(PUNICODE_STRING full_path, PUNICODE_STRING base_path)
{
	USHORT length = full_path->Length / sizeof(WCHAR);
	PWCH buffer = full_path->Buffer;

	USHORT base_length = 0;
	PWCH base_buffer_start = buffer;

	USHORT idx = length;

	do
	{
		--idx;

		if (buffer[idx] == L'\\')
			break;

		base_length++;
		base_buffer_start = &buffer[idx];

	} while (idx > 0);

	base_path->Length = base_length * sizeof(WCHAR);
	base_path->MaximumLength = base_length * sizeof(WCHAR);
	base_path->Buffer = base_buffer_start;
}

NTSTATUS findProcessByName(const PWCHAR process_name, PEPROCESS& peprocess_out)
{
	UNICODE_STRING process_name_unicode = { 0 };
	RtlInitUnicodeString(&process_name_unicode, process_name);

	if (DEBUG_GET_PROCESS)
		debug("[+] Getting process by name: %wZ\n", process_name_unicode);

	const auto activeProcessLinksOffset = processGetActiveProcessLinksOffset();
	const auto imageFilePtrOffset = processGetImageFilePtrOffset();

	const PLIST_ENTRY start_address = PLIST_ENTRY((uintptr_t)PsInitialSystemProcess + activeProcessLinksOffset);

	if (DEBUG_GET_PROCESS)
		debug("[ ]   Start address: 0x%p\n", start_address);

	for (PLIST_ENTRY entry = start_address;
		entry != nullptr && entry->Flink != start_address;
		entry = entry->Flink)
	{
		PEPROCESS pEprocess = (PEPROCESS)((uintptr_t)entry - activeProcessLinksOffset);

		if (DEBUG_GET_PROCESS)
			debug("[ ]   Checking eprocess at 0x%p\n", pEprocess);

		PFILE_OBJECT file_object = *(PFILE_OBJECT*)((uintptr_t)pEprocess + imageFilePtrOffset);

		if (file_object == nullptr)
		{
			if (DEBUG_GET_PROCESS)
				debug("[~]   File object is null.\n");

			continue;
		}

		if (DEBUG_GET_PROCESS)
			debug("[ ]   Getting file name of process at 0x%p\n", file_object);

		UNICODE_STRING base_path = { 0 };
		GetBasePathUnicodeString(&file_object->FileName, &base_path);

		if (DEBUG_GET_PROCESS)
			debug("[ ]   Checking process name %wZ (%wZ)\n", &base_path, &file_object->FileName);

		if (RtlEqualUnicodeString(&base_path, &process_name_unicode, TRUE))
		{
			ObReferenceObject(pEprocess);

			peprocess_out = pEprocess;

			if (DEBUG_GET_PROCESS)
				debug("[+]   Found process %wZ at address 0x%p\n", &process_name_unicode, peprocess_out);

			return STATUS_SUCCESS;
		}
	}

	peprocess_out = nullptr;

	if (DEBUG_GET_PROCESS)
		debug("[-]   Unable to find process %wZ\n", &process_name_unicode);

	return STATUS_UNSUCCESSFUL;
}
