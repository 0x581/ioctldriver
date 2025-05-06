#ifndef ASSPILEDRIVER_H_
#define ASSPILEDRIVER_H_

#ifdef _WIN32
#ifdef MYDRIVER_EXPORTS
#define ASSPILEDRIVER_API __declspec(dllexport)
#else
#define ASSPILEDRIVER_API __declspec(dllimport)
#endif
#else // For GCC/Clang on Linux/macOS
#define ASSPILEDRIVER_API __attribute__((visibility("default")))
#endif

#include <crtdefs.h>

#include "debug_flags.h"


ASSPILEDRIVER_API bool do_nothing(void* driver_handle);

ASSPILEDRIVER_API bool attach_to_process(void* driver_handle, const wchar_t* process_name);

ASSPILEDRIVER_API bool unattach_from_process(void* driver_handle);

ASSPILEDRIVER_API bool process_is_alive(void* driver_handle);

ASSPILEDRIVER_API uintptr_t get_module_base(void* driver_handle, const wchar_t* module_name);

ASSPILEDRIVER_API size_t set_debug_flags(void* driver_handle, DebugFlags new_flags);

ASSPILEDRIVER_API size_t read_memory(void* driver_handle, const uintptr_t addr, void* buffer, const size_t size);

ASSPILEDRIVER_API bool write_memory(void* driver_handle, const uintptr_t addr, void* buffer, const size_t size);

#endif // ASSPILEDRIVER_H_
