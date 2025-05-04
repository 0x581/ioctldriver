#pragma once

#include <ntifs.h>

class Process {
public:
	PEPROCESS eprocess;
	HANDLE pid;

	uintptr_t cr3;
	PPEB pPeb;
private:
	PLONG exit_status;

	uintptr_t process_section_base_address;

public:
	Process();

	bool Attached() const;

	void Connect(PEPROCESS target_process);

	void Clear();

	bool is_alive() const;
};
