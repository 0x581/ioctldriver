#pragma once

#include "kdebug.h"

#ifdef _DEBUG
#include <ntifs.h>
#include <ntstrsafe.h>
#else
#include <ntdef.h>
#endif

void debug(PCSTR format, ...) {
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(format);
#else
	va_list args;
	va_start(args, format);

	char buffer[1024];

	const auto ret = RtlStringCbVPrintfA(buffer, sizeof(buffer), format, args);

	if (NT_SUCCESS(ret))
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, buffer));
	else
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Error in debug: formatting failed\n"));

	va_end(args);
#endif // DEBUG
}

DebugFlags DEBUG_FLAGS = DEFAULT_FLAGS;
