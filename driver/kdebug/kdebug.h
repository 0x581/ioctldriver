#pragma once

#include "debug_flags.h"

extern DebugFlags DEBUG_FLAGS;

#define DEBUG_DEV_CTRL				(DEBUG_FLAGS.flags.device_control)
#define DEBUG_ATTACH				(DEBUG_FLAGS.flags.attach)
#define DEBUG_DETACH				(DEBUG_FLAGS.flags.detach)
#define DEBUG_PROCESS_STRUCT		(DEBUG_FLAGS.flags.process_struct)
#define DEBUG_TARGET_ALIVE			(DEBUG_FLAGS.flags.target_alive)

#define DEBUG_READ					(DEBUG_FLAGS.flags.read)
#define DEBUG_WRITE					(DEBUG_FLAGS.flags.write)
#define DEBUG_LINEAR_TRANSLATION	(DEBUG_FLAGS.flags.linear_translation)

#define DEBUG_GET_PROCESS			(DEBUG_FLAGS.flags.get_process)
#define DEBUG_GET_MODULE			(DEBUG_FLAGS.flags.get_module)

#define DEBUG_SET_DEBUG_FLAGS		(DEBUG_FLAGS.flags.set_debug_level)


void debug(const char* format, ...);
