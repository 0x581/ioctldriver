#pragma once

#include "driver_functions.h"
#include <kdebug.h>


NTSTATUS is_process_alive(Process& process, RequestIsProcessAlive* request) {
	if (DEBUG_TARGET_ALIVE)
		debug("[+] Checking if target process is alive.\n");

	if (!process.Attached())
	{
		if (DEBUG_TARGET_ALIVE)
			debug("[-] Not attached to a process.\n");

		request->is_alive = false;

		return STATUS_INVALID_PARAMETER;
	}

	request->is_alive = process.is_alive();

	if (DEBUG_TARGET_ALIVE)
		debug("[+] Target process is %s.\n", request->is_alive ? "alive" : "dead");

	return STATUS_SUCCESS;
}

