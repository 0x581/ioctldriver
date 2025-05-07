#pragma once

#include "driver_functions.h"
#include <kdebug.h>

#include "../process/includes/get_process.h"


static bool check_arguments(RequestAttach* request) {
	const auto length = wcsnlen_s(request->process_name, sizeof(request->process_name) / sizeof(wchar_t));

	if (length == 0 || length > MAX_PATH)
		return false;

	return true;
}

NTSTATUS attach(Process& process, RequestAttach* request) {
	if (process.Attached())
		detach(process);

	if (!check_arguments(request))
	{
		if (DEBUG_ATTACH)
			debug("[-] Invalid process name.\n");

		return STATUS_INVALID_PARAMETER;
	}

	const PWCHAR process_name = request->process_name;

	if (DEBUG_ATTACH)
		debug("[+] Attaching to process: %ws\n", process_name);

	PEPROCESS target_process = nullptr;
	NTSTATUS status = findProcessByName(process_name, target_process);

	if (NT_SUCCESS(status))
		process.Connect(target_process);

	request->pid = process.pid;

	if (DEBUG_ATTACH)
	{
		if (NT_SUCCESS(status))
			debug("[+] Attached to process: %ws (PID %d)\n", process_name, process.pid);
		else
			debug("[-] Failed to attach to process: %ws\n", process_name);
	}

	return status;
}
