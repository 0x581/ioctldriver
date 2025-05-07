#pragma once

#include "driver_functions.h"
#include <kdebug.h>


NTSTATUS detach(Process& process) {
	if (DEBUG_UNATTACH)
		debug("[+] Unattaching from process.\n");

	if (!process.Attached())
	{
		if (DEBUG_UNATTACH)
			debug("[-] Not attached to a process.\n");
		return STATUS_INVALID_PARAMETER;
	}

	process.Clear();

	if (DEBUG_UNATTACH)
		debug("[+] Unattached from process.\n");

	return STATUS_SUCCESS;
}
