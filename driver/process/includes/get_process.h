#pragma once

#include <ntifs.h>

NTSTATUS findProcessIdByName(const PWCHAR process_name, HANDLE& pid);
