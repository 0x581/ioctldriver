#pragma once

#include <ntifs.h>

#include "includes/get_module.h"
#include "includes/get_process.h"

class Process {
public:
	PEPROCESS eprocess;
	HANDLE pid;

	uintptr_t cr3;
private:
	PLONG exit_status;

	uintptr_t process_section_base_address;
public:
	Process();

	bool Connected() const;

	void Connect(PEPROCESS target_process);

	void Clear();

	bool is_alive() const;
};
