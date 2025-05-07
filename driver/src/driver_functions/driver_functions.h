#pragma once

#include "../process/process.h"
#include "requests.h"

#include <ntdef.h>

#define USE_MANUAL_READ
#define USE_MANUAL_WRITE


NTSTATUS attach(Process& process, RequestAttach* request);
NTSTATUS detach(Process& process);

NTSTATUS is_process_alive(Process& process, RequestIsProcessAlive* request);

NTSTATUS read_memory(Process& process, RequestReadMemory* request);
NTSTATUS write_memory(Process& process, RequestWriteMemory* request);

NTSTATUS get_module_base(Process& process, RequestGetModuleBase* request);

NTSTATUS set_debug_flags(RequestSetDebugFlags* request);
