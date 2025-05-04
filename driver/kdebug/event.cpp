#pragma once

#include <ntddk.h>
#include <ntstrsafe.h> // For RtlStringCchPrintfW

//---------------------------------------------------------------------------
// Function to write an informational or error message to the System Event Log
//---------------------------------------------------------------------------
VOID WriteToEventLog(
	_In_ PVOID      IoObject,       // DriverObject or DeviceObject
	_In_ ULONG      EventID,        // Your custom Event ID
	_In_ NTSTATUS   Status,         // Optional status code to include
	_In_opt_ PCWSTR InsertionString // Optional custom string
) {
	PIO_ERROR_LOG_PACKET errorLogEntry = NULL;
	ULONG                packetSize = sizeof(IO_ERROR_LOG_PACKET);
	ULONG                stringSize = 0;
	ULONG                statusStringSize = 0;
	WCHAR                statusString[32]; // "Status=0x12345678"
	ULONG                numStrings = 0;

	// --- Calculate required size ---

	// Size for the optional custom insertion string
	if (InsertionString != NULL)
	{
		// Include space for the null terminator
		stringSize = ((ULONG)wcslen(InsertionString) + 1) * sizeof(WCHAR);
		packetSize += stringSize;
		numStrings++;
	}

	// Size for the NTSTATUS string (always include for simplicity here)
	// Format it now to know the exact size needed.
	NTSTATUS formatStatus = RtlStringCchPrintfW(
		statusString,
		RTL_NUMBER_OF(statusString),
		L"Status=0x%08X", // Format NTSTATUS as hex
		Status
	);

	if (NT_SUCCESS(formatStatus))
	{
		// Include space for the null terminator
		statusStringSize = ((ULONG)wcslen(statusString) + 1) * sizeof(WCHAR);
		packetSize += statusStringSize;
		numStrings++;
	}
	else
	{
		// Handle potential formatting error (optional)
		statusStringSize = 0; // Don't include it if formatting failed
	}


	// --- Allocate the log entry ---
	// Use IoAllocateErrorLogEntry; it's designed for this.
	// Pass either DriverObject or DeviceObject.
	errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
		IoObject,
		(UCHAR)packetSize
	);

	if (errorLogEntry == NULL)
	{
		KdPrint(("MyDriver: Failed to allocate error log entry\n"));
		// Consider ExAllocatePool/ExFreePool as a fallback if needed,
		// but IoAllocateErrorLogEntry is preferred.
		return;
	}

	// --- Fill the log entry packet ---
	RtlZeroMemory(errorLogEntry, packetSize);

	errorLogEntry->ErrorCode = EventID; // Your custom event ID
	errorLogEntry->NumberOfStrings = (USHORT)numStrings;
	errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
	// errorLogEntry->EventCategory = 0; // Optional category
	// errorLogEntry->DumpDataSize = 0; // No binary data for this example
	errorLogEntry->FinalStatus = Status; // Store the NTSTATUS here too

	// --- Copy insertion strings into the packet ---
	// The strings go immediately after the IO_ERROR_LOG_PACKET structure
	PUCHAR currentStringPtr = (PUCHAR)errorLogEntry + sizeof(IO_ERROR_LOG_PACKET);

	// Copy the formatted status string (if successful)
	if (statusStringSize > 0)
	{
		RtlCopyMemory(currentStringPtr, statusString, statusStringSize);
		currentStringPtr += statusStringSize; // Move pointer past this string
	}

	// Copy the custom insertion string (if provided)
	if (InsertionString != NULL && stringSize > 0)
	{
		RtlCopyMemory(currentStringPtr, InsertionString, stringSize);
		// No need to advance pointer further if it's the last string
	}


	// --- Write the entry to the System log ---
	// The system will free the allocated entry automatically.
	IoWriteErrorLogEntry(errorLogEntry);

	KdPrint(("MyDriver: Logged EventID 0x%X, Status 0x%X\n", EventID, Status));
}
