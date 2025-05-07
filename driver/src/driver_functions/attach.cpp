#pragma once

#include "driver_functions.h"
#include <kdebug.h>

#include "../../process/includes/get_process.h"


NTSTATUS attach(Process& process, RequestAttach* request) {
	if (process.Attached())
		detach(process);

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
