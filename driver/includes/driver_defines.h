#pragma once

#include <crtdefs.h>

#define DRIVER_NAME "AssPileDriver"

constexpr const wchar_t* device_name_w = L"\\Device\\" DRIVER_NAME;

constexpr const wchar_t* driver_file_w = L"\\\\?\\GLOBALROOT\\Device\\" DRIVER_NAME;
