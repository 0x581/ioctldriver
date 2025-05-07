#pragma once

#include "driver_functions.h"
#include <kdebug.h>


NTSTATUS detach(Process& process) {
	if (DEBUG_DETACH)
		debug("[+] Detaching from process.\n");

	if (!process.Attached())
	{
		if (DEBUG_DETACH)
			debug("[-] Not attached to a process.\n");
		return STATUS_INVALID_PARAMETER;
	}

	process.Clear();

	if (DEBUG_DETACH)
		debug("[+] Detached from process.\n");

	return STATUS_SUCCESS;
}
