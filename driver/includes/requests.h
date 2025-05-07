#pragma once

constexpr auto MAX_PATH = 260;

struct Request {};

typedef Request RequestVerifyConnection;

struct RequestAttach {
	wchar_t process_name[MAX_PATH + 1];
	HANDLE pid;
};

typedef Request RequestUnattach;

struct RequestIsProcessAlive {
	bool is_alive;
};

struct RequestReadMemory {
	void* source;
	void* destination;

	size_t size;
	size_t return_size;
};

typedef RequestReadMemory RequestWriteMemory;

struct RequestGetModuleBase {
	wchar_t module_name[MAX_PATH + 1];
	uintptr_t module_base;
};

struct RequestSetDebugFlags {
	size_t new_flags;
	size_t old_flags;
};
