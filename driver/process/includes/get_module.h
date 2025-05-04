#pragma once

#include <ntifs.h>

NTSTATUS findModuleBaseAddress(const PEPROCESS process, const PWCHAR module_name, uintptr_t& address);