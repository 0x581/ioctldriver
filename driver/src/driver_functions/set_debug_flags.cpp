#pragma once

#include "driver_functions.h"
#include <kdebug.h>


NTSTATUS set_debug_flags(RequestSetDebugFlags* request) {
	request->old_flags = DEBUG_FLAGS.raw;

	DEBUG_FLAGS.raw = request->new_flags;

	if (DEBUG_SET_DEBUG_FLAGS)
		debug("[+] Debug flags updated from %p to %p.\n", request->old_flags, DEBUG_FLAGS.raw);

	return STATUS_SUCCESS;
}
