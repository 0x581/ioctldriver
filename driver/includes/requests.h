#pragma once

struct Request {
	void* pid;

	void* address;
	void* buffer;

	size_t value;
	size_t return_value;

	size_t size;
	size_t return_size;
};
