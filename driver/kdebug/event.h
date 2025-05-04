#pragma once

#include <ntddk.h>

VOID WriteToEventLog(
	_In_ PVOID      IoObject,       // DriverObject or DeviceObject
	_In_ ULONG      EventID,        // Your custom Event ID
	_In_ NTSTATUS   Status,         // Optional status code to include
	_In_opt_ PCWSTR InsertionString // Optional custom string
);
