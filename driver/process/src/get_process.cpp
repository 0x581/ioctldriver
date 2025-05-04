#pragma once

#include "../includes/get_process.h"

#include "../includes/offsets.h"

#include "kdebug.h"

void GetBasePathUnicodeString(PUNICODE_STRING full_path, PUNICODE_STRING base_path) {
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

NTSTATUS findProcessByName(const PWCHAR process_name, PEPROCESS& peprocess_out) {
	UNICODE_STRING process_name_unicode = { 0 };
	RtlInitUnicodeString(&process_name_unicode, process_name);

	const auto activeProcessLinksOffset = processGetActiveProcessLinksOffset();
	const auto imageFilePtrOffset = processGetImageFilePtrOffset();

	const PLIST_ENTRY start_address = PLIST_ENTRY((uintptr_t)PsInitialSystemProcess + activeProcessLinksOffset);

	PLIST_ENTRY entry = start_address;

	while (entry != nullptr && entry->Flink != start_address)
	{
		PEPROCESS pEprocess = (PEPROCESS)((uintptr_t)entry - activeProcessLinksOffset);

		PFILE_OBJECT file_object = (PFILE_OBJECT)((uintptr_t)pEprocess + imageFilePtrOffset);

		UNICODE_STRING base_path = { 0 };
		GetBasePathUnicodeString(&file_object->FileName, &base_path);

		if (RtlEqualUnicodeString(&base_path, &process_name_unicode, TRUE))
		{
			ObReferenceObject(pEprocess);

			peprocess_out = pEprocess;

			if (DEBUG_GET_MODULE)
				debug("[+] Found process %wZ at address 0x%llX\n", &process_name_unicode, peprocess_out);

			return STATUS_SUCCESS;
		}

		entry = entry->Flink;
	}

	peprocess_out = nullptr;

	if (DEBUG_GET_MODULE)
		debug("[-] Unable to find process %wZ\n", &process_name_unicode);

	return STATUS_UNSUCCESSFUL;
}
