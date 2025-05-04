#pragma once

#include <ntifs.h>

NTSTATUS findProcessByName(const PWCHAR process_name, PEPROCESS& peprocess_out);
