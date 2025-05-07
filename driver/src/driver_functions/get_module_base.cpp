#pragma once

#include "driver_functions.h"
#include <kdebug.h>

#include "../../process/includes/get_module.h"


NTSTATUS get_module_base(Process& process, RequestGetModuleBase* request) {
	return findModuleBaseAddress(process, request->module_name, request->module_base);
}
