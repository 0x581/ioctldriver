#pragma once

#ifndef DEBUG
#define DEBUG
#endif

#include <ntdef.h>

constexpr auto DEBUG_FLAG_DEV_CTRL = 0x1 << 0;
constexpr auto DEBUG_FLAG_ATTACH = 0x1 << 1;
constexpr auto DEBUG_FLAG_UNATTACH = 0x1 << 2;
constexpr auto DEBUG_FLAG_TARGET_ALIVE = 0x1 << 3;
constexpr auto DEBUG_FLAG_READ = 0x1 << 10;
constexpr auto DEBUG_FLAG_WRITE = 0x1 << 11;

constexpr auto DEBUG_FLAGS = (
	0x0
	//| DEBUG_FLAG_DEV_CTRL
	| DEBUG_FLAG_ATTACH
	| DEBUG_FLAG_UNATTACH
	| DEBUG_FLAG_TARGET_ALIVE
	//| DEBUG_FLAG_READ
	//| DEBUG_FLAG_WRITE
	);

constexpr auto DEBUG_DEV_CTRL = DEBUG_FLAGS & DEBUG_FLAG_DEV_CTRL;
constexpr auto DEBUG_ATTACH = DEBUG_FLAGS & DEBUG_FLAG_ATTACH;
constexpr auto DEBUG_UNATTACH = DEBUG_FLAGS & DEBUG_FLAG_UNATTACH;
constexpr auto DEBUG_TARGET_ALIVE = DEBUG_FLAGS & DEBUG_FLAG_TARGET_ALIVE;
constexpr auto DEBUG_READ = DEBUG_FLAGS & DEBUG_FLAG_READ;
constexpr auto DEBUG_WRITE = DEBUG_FLAGS & DEBUG_FLAG_WRITE;


void debug_print(PCSTR format, ...);
