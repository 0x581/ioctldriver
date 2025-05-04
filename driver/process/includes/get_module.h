#pragma once

#include <ntifs.h>

#include "process_struct.h"

NTSTATUS findModuleBaseAddress(Process& process, PCWCHAR module_name, uintptr_t& address);
