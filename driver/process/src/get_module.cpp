#pragma once

#include "../includes/get_module.h"

NTSTATUS findModuleBaseAddress(const PEPROCESS process, const PWCHAR module_name, uintptr_t& address) {
NTSTATUS findModuleBaseAddress(Process& process, PCWCHAR module_name, uintptr_t& address) {
	UNREFERENCED_PARAMETER(process);
	UNREFERENCED_PARAMETER(module_name);
	UNREFERENCED_PARAMETER(address);

	return STATUS_NOT_IMPLEMENTED;
}
