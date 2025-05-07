#pragma once

#include "../includes/get_module.h"

#include "../includes/virtual_to_physical.h"
#include "../includes/offsets.h"

#include "kdebug.h"

#define MAX_PATH 260

struct LDR_DATA_TABLE_ENTRY {
	struct _LIST_ENTRY InLoadOrderLinks;                                    //0x0
	struct _LIST_ENTRY InMemoryOrderLinks;                                  //0x10
	struct _LIST_ENTRY InInitializationOrderLinks;                          //0x20
	VOID* DllBase;                                                          //0x30
	VOID* EntryPoint;                                                       //0x38
	ULONG SizeOfImage;                                                      //0x40
	struct _UNICODE_STRING FullDllName;                                     //0x48
	struct _UNICODE_STRING BaseDllName;                                     //0x58
	// ...
};


NTSTATUS findModuleBaseAddress(Process& process, PCWCHAR module_name, uintptr_t& address) {
	UNICODE_STRING module_name_unicode = { 0 };
	RtlInitUnicodeString(&module_name_unicode, module_name);

	uintptr_t ldrDataPtr = ReadProcessMemory<uintptr_t>(process, (uintptr_t)process.pPeb + pebLdrOffset);

	if (!ldrDataPtr)
	{
		if (DEBUG_GET_MODULE)
			debug("[-] Failed to read LdrDataPtr.\n");
		return STATUS_UNSUCCESSFUL;
	}

	const PLIST_ENTRY start_address = PLIST_ENTRY(ldrDataPtr + ldrInLoadOrderModuleListOffset);

	LDR_DATA_TABLE_ENTRY entry = { 0 };
	ReadProcessMemory(process, (PVOID)start_address, &entry, sizeof(PLIST_ENTRY));

	WCHAR buffer[MAX_PATH + 1] = { 0 };

	do
	{
		entry = ReadProcessMemory<LDR_DATA_TABLE_ENTRY>(process, entry.InLoadOrderLinks.Flink);

		UNICODE_STRING current_module_name{
			.Length = entry.BaseDllName.Length,
			.MaximumLength = entry.BaseDllName.MaximumLength,
			.Buffer = buffer,
		};

		if (current_module_name.Length > MAX_PATH)
		{
			if (DEBUG_GET_MODULE)
				debug("[-] Module name is too long.\n");
			continue;
		}

		size_t bytes_read = 0;

		ReadProcessMemory(process, entry.BaseDllName.Buffer, current_module_name.Buffer,
			current_module_name.Length, &bytes_read);

		if (bytes_read != current_module_name.Length)
		{
			if (DEBUG_GET_MODULE)
				debug("[-] Failed to read Unicode string.\n");

			continue;
		}

		current_module_name.Buffer[current_module_name.Length / sizeof(WCHAR)] = 0;

		if (RtlCompareUnicodeString(&current_module_name, &module_name_unicode, TRUE) == 0)
		{
			address = (uintptr_t)entry.DllBase;
			if (DEBUG_GET_MODULE)
				debug("[+] Found module %wZ at address 0x%llX\n", &current_module_name, address);

			return STATUS_SUCCESS;
		}
	} while (entry.InLoadOrderLinks.Flink != start_address);

	if (DEBUG_GET_MODULE)
		debug("[-] Unable to find module %wZ\n", &module_name_unicode);

	return STATUS_UNSUCCESSFUL;
}
