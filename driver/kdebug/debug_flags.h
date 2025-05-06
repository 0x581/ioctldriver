#pragma once

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
				.target_alive = 1,
						.read = 0,
					   .write = 0,
		  .linear_translation = 0,
				 .get_process = 0,
				  .get_module = 0,
			 .set_debug_level = 1,
	}
};
