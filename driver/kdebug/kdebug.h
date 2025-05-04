#pragma once

#ifndef DEBUG
#define DEBUG
#endif

union DebugFlags {
	size_t raw;

	struct {
		size_t device_control : 1;
		//
		// Initialization/cleanup
		size_t attach : 1;
		size_t unattach : 1;
		size_t process_struct : 1;
		size_t target_alive : 1;
		//
		// Virtual memory operations
		size_t read : 1;
		size_t write : 1;
		size_t linear_translation : 1;
		//
		// Walking process structures
		size_t get_process : 1;
		size_t get_module : 1;
		//
		// Debug Flags
		size_t set_debug_level : 1;

		// Reserved for future use
		size_t reserved : 53;
	} flags;
};


constexpr DebugFlags DEFAULT_FLAGS = {
	.flags {
			  .device_control = 0,
					  .attach = 1,
					.unattach = 1,
			  .process_struct = 1,
				.target_alive = 0,
						.read = 0,
					   .write = 0,
		  .linear_translation = 0,
				 .get_process = 1,
				  .get_module = 1,
			 .set_debug_level = 1,
	}
};

static volatile DebugFlags DEBUG_FLAGS = DEFAULT_FLAGS;

#define DEBUG_DEV_CTRL				(DEBUG_FLAGS.flags.device_control)
#define DEBUG_ATTACH				(DEBUG_FLAGS.flags.attach)
#define DEBUG_UNATTACH				(DEBUG_FLAGS.flags.unattach)
#define DEBUG_PROCESS_STRUCT		(DEBUG_FLAGS.flags.process_struct)
#define DEBUG_TARGET_ALIVE			(DEBUG_FLAGS.flags.target_alive)

#define DEBUG_READ					(DEBUG_FLAGS.flags.read)
#define DEBUG_WRITE					(DEBUG_FLAGS.flags.write)
#define DEBUG_LINEAR_TRANSLATION	(DEBUG_FLAGS.flags.linear_translation)

#define DEBUG_GET_PROCESS			(DEBUG_FLAGS.flags.get_process)
#define DEBUG_GET_MODULE			(DEBUG_FLAGS.flags.get_module)

#define DEBUG_SET_DEBUG_FLAGS		(DEBUG_FLAGS.flags.set_debug_level)


void debug(const char* format, ...);
