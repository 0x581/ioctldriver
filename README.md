# ioctldriver (`AssPileDriver`)

[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-blue.svg)](https://microsoft.com/windows)
[![Driver](https://img.shields.io/badge/Type-Kernel--Mode%20Driver-red.svg)](#)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-brightgreen.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A Windows x64 Kernel-Mode Driver and User-Mode Client Library (`apd_api`) designed for low-level process memory inspection, physical address mapping, and module resolution via an IOCTL control interface.

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Architecture & Repository Structure](#architecture--repository-structure)
- [IOCTL Protocol & Control Codes](#ioctl-protocol--control-codes)
- [Dynamic Offset Support](#dynamic-offset-support)
- [Prerequisites](#prerequisites)
- [Building the Project](#building-the-project)
- [Usage & Deployment](#usage--deployment)
- [C++ User-Mode API Reference](#c-user-mode-api-reference)
- [Disclaimer & License](#disclaimer--license)

---

## Overview

`ioctldriver` exposes a communication channel (`\\Device\\AssPileDriver`) allowing authorized user-mode applications to perform advanced process inspection tasks without relying on standard user-mode debugging APIs (such as `OpenProcess` or `ReadProcessMemory`).

Rather than depending on handle-based Win32 APIs, the driver interacts directly with kernel structures and hardware paging tables to read and write target process memory.

---

## Key Features

- **Manual CR3 & Page-Table Walking:**
  Translates virtual addresses to physical addresses directly via CR3 / Directory Table Base traversal:
  - Supports **4 KB** standard pages (`PML4` &rarr; `PDPT` &rarr; `PD` &rarr; `PT`).
  - Supports **2 MB** large pages.
  - Supports **1 GB** huge pages.
  - Physical memory access via `MmCopyMemory` (`MM_COPY_MEMORY_PHYSICAL`) and `MmMapIoSpaceEx`.
  - Fallback toggle to `MmCopyVirtualMemory` available via `#ifndef USE_MANUAL_READ` / `#ifndef USE_MANUAL_WRITE`.

- **Dynamic Windows Offset Resolution:**
  Automatically detects host build numbers using `RtlGetVersion` and calculates the appropriate offsets inside `EPROCESS` across major Windows releases:
  - Windows 10 (1803, 1809, 1903, 1909, 2004, 20H2, 22H2)
  - Windows 11 (23H2, 24H2)
  - Automatically locates `UniqueProcessId`, `ActiveProcessLinks`, `UserDirectoryTableBase`, `SectionBaseAddress`, `Peb`, and `ExitStatus`.

- **Kernel-Side Module Enumeration:**
  Traverses the target process `_PEB_LDR_DATA` (`InLoadOrderModuleList`) directly to find the base address of any loaded DLL or executable module.

- **Granular Bitmask Debug System:**
  Fine-grained runtime debug flags allow selective logging via `DbgPrintEx` for:
  - Device control dispatches
  - Attach / Detach operations
  - Memory reads and writes
  - Linear address translations
  - Process and module lookups

- **Clean C++ User-Mode Library (`apd_api`):**
  A statically linkable C++20 library encapsulating IOCTL requests into simple, idiomatic functions (`apd::open_driver()`, `apd::attach_to_process()`, `apd::read_memory()`, `apd::write_memory()`, etc.).

---

## Architecture & Repository Structure

```text
ioctldriver/
├── driver/                 # Windows Kernel-Mode Driver (KMDF / WDM)
│   ├── includes/           # Common IOCTL codes, requests, and defines
│   │   ├── driver_defines.h
│   │   ├── ioctl_codes.h
│   │   └── requests.h
│   ├── kdebug/             # Kernel debugging & event log utilities
│   ├── src/
│   │   ├── main.cpp        # DriverEntry, Device dispatch & IRP handling
│   │   ├── driver_functions/  # Core handlers (attach, read, write, etc.)
│   │   └── process/        # Process list walker, CR3 resolution, paging
│   └── driver.vcxproj
│
├── apd_api/                # User-Mode Static Library (C++20)
│   ├── asspiledriver.h     # High-level client API header
│   ├── asspiledriver.cpp   # DeviceIoControl wrappers
│   └── apd_api.vcxproj
│
├── um/                     # User-Mode Console Client (Sample / Test CLI)
│   ├── main.cpp            # Demonstration & benchmark tests
│   └── um.vcxproj
│
├── ioctldriver.sln         # Visual Studio 2022 Solution
└── README.md
```

---

## IOCTL Protocol & Control Codes

All communications use buffered I/O (`METHOD_BUFFERED` / `DO_BUFFERED_IO`) targeting device `FILE_DEVICE_UNKNOWN`:

| IOCTL Code | Control Macro / Constant | Description | Payload Structure |
|---|---|---|---|
| `CTL_CODE(0x22, 0, ...)` | `ioctl_codes::verify_connection` | Tests whether driver device is open and responsive. | `RequestVerifyConnection` |
| `CTL_CODE(0x22, 10, ...)` | `ioctl_codes::attach` | Attaches to target process by executable name (`PEPROCESS`). | `RequestAttach` |
| `CTL_CODE(0x22, 11, ...)` | `ioctl_codes::detach` | Detaches and cleans up stored process references. | `RequestDetach` |
| `CTL_CODE(0x22, 20, ...)` | `ioctl_codes::is_process_alive` | Checks if target process is currently active. | `RequestIsProcessAlive` |
| `CTL_CODE(0x22, 100, ...)` | `ioctl_codes::read_memory` | Reads virtual memory via physical page translation. | `RequestReadMemory` |
| `CTL_CODE(0x22, 101, ...)` | `ioctl_codes::write_memory` | Writes virtual memory via physical page translation. | `RequestWriteMemory` |
| `CTL_CODE(0x22, 400, ...)` | `ioctl_codes::get_module_base` | Enumerates PEB loader data to find a module base address. | `RequestGetModuleBase` |
| `CTL_CODE(0x22, 500, ...)` | `ioctl_codes::set_debug_flags` | Sets runtime kernel debug print bitmask flags. | `RequestSetDebugFlags` |

---

## Dynamic Offset Support

The driver dynamically queries the NT build number at runtime and chooses the corresponding field offsets in `EPROCESS`:

| Windows Version | Build Number | PID Offset | ActiveLinks | UserDirectoryTableBase | SectionBaseAddress | PEB Pointer | ExitStatus |
|---|---|---|---|---|---|---|---|
| Windows 10 1803 | 17134 | `0x2E0` | `0x2E8` | `0x278` | `0x520` | `0x624` | `0x624` |
| Windows 10 1809 | 17763 | `0x2E0` | `0x2E8` | `0x278` | `0x520` | `0x624` | `0x624` |
| Windows 10 1903/1909 | 18362/18363 | `0x2E8` | `0x2F0` | `0x280` | `0x520` | `0x3F8` | `0x654` |
| Windows 10 2004 - 22H2 | 19041 - 19045 | `0x440` | `0x448` | `0x388` | `0x520` | `0x550` | `0x7D4` |
| Windows 11 23H2 | 22621/22631 | `0x440` | `0x448` | `0x388` | `0x520` | `0x550` | `0x7D4` |
| Windows 11 24H2 | 26100+ | `0x1D0` | `0x1D8` | `0x158` | `0x2B0` | `0x2E0` | `0x554` |

---

## Prerequisites

To build and run this project, make sure you have:

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Workload: *Desktop development with C++*
   - Component: *MSVC v143 - VS 2022 C++ x64/x86 build tools*
   - C++20 Language Standard
2. **Windows 10/11 SDK & Windows Driver Kit (WDK):**
   - Matching your target Windows OS build (e.g. WDK 10.0.26100.0 or Windows 10 WDK).
3. **Target System for Testing:**
   - A secondary testing PC or Virtual Machine (VMware, VirtualBox, Hyper-V) with **TESTSIGNING** enabled or a kernel debugger attached.

---

## Building the Project

### Using Visual Studio:
1. Open `ioctldriver.sln` in Visual Studio 2022.
2. Select configuration: **`Release`** and platform: **`x64`**.
3. Right-click the Solution and select **Build Solution** (or press `Ctrl+Shift+B`).

### Using MSBuild via Command Line:
```powershell
# Open Developer Command Prompt or PowerShell
msbuild ioctldriver.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild
```

The build outputs will be placed in the `build/` directory:
- `build/executables/driver.sys` - The compiled kernel-mode driver binary.
- `build/api/apd_api.lib` - The user-mode client static library.
- `build/executables/um.exe` - The user-mode CLI test application.

---

## Usage & Deployment

> [!WARNING]
> Because 64-bit Windows enforces Driver Signature Enforcement (DSE), you must run the driver either in Test Signing mode or via your preferred driver loading method (e.g. OSR Driver Loader, KDMapper, or manual mapping).

### 1. Enable Test Mode (on Test VM):
Run from an elevated Command Prompt (`cmd.exe` as Administrator):
```cmd
bcdedit /set testsigning on
```
Restart your computer/VM to apply changes.

### 2. Create and Start the Service:
```cmd
# Create the service pointing to driver.sys
sc.exe create AssPileDriver binPath= "C:\path\to\driver.sys" type= kernel

# Start the driver service
sc.exe start AssPileDriver
```

### 3. Run the User-Mode Client:
Launch `um.exe` as Administrator:
```cmd
um.exe
```

### 4. Stop and Remove the Driver Service:
```cmd
sc.exe stop AssPileDriver
sc.exe delete AssPileDriver
```

---

## C++ User-Mode API Reference

Link against `apd_api.lib` and include `asspiledriver.h`:

```cpp
#include "asspiledriver.h"
#include <iostream>

int main() {
    // 1. Open a handle to the driver
    HANDLE driver = apd::open_driver();
    if (driver == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open driver device.\n";
        return 1;
    }

    // 2. Attach to target process by executable name
    HANDLE pid = nullptr;
    if (apd::attach_to_process(driver, L"target.exe", &pid)) {
        std::cout << "Attached to target.exe with PID: " << pid << "\n";

        // 3. Query module base address
        uintptr_t module_base = apd::get_module_base(driver, L"target.exe");
        std::cout << "target.exe base: 0x" << std::hex << module_base << "\n";

        // 4. Read memory
        int value = 0;
        size_t bytes_read = apd::read_memory(driver, module_base + 0x1000, &value, sizeof(value));

        // 5. Write memory
        int new_value = 1337;
        size_t bytes_written = apd::write_memory(driver, module_base + 0x1000, &new_value, sizeof(new_value));

        // 6. Check if target process is still running
        if (apd::is_process_alive(driver)) {
            std::cout << "Process is active.\n";
        }

        // 7. Detach
        apd::detach_from_process(driver);
    }

    // 8. Close driver handle
    apd::close_driver(driver);
    return 0;
}
```

---

## Disclaimer & License

### Disclaimer
This software is provided for **educational and security research purposes only**. Running kernel drivers can cause system instability (BSOD) if improperly configured or used with unsupported Windows builds. The author is not responsible for any misuse or damage caused by this software.

### License
This project is licensed under the [MIT License](LICENSE).
