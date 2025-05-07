#pragma once

#include "driver_functions.h"
#include <kdebug.h>

#include "../process/includes/get_module.h"


static bool check_arguments(RequestGetModuleBase* request) {
	const auto length = wcsnlen_s(request->module_name, sizeof(request->module_name) / sizeof(wchar_t));

	if (length == 0 || length > MAX_PATH)
		return false;

	return true;
}

NTSTATUS get_module_base(Process& process, RequestGetModuleBase* request) {
	if (!check_arguments(request))
	{
		if (DEBUG_GET_MODULE)
			debug("[-] Invalid module name.\n");

		return STATUS_INVALID_PARAMETER;
	}

	return findModuleBaseAddress(process, request->module_name, request->module_base);
}
