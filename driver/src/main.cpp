#pragma once

#include "driver_defines.hpp"

#include <ntifs.h>

#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG
	#include <ntstrsafe.h>
#endif

constexpr auto DEBUG_FLAG_DEV_CTRL = 0x1 << 0;
constexpr auto DEBUG_FLAG_ATTACH = 0x1 << 1;
constexpr auto DEBUG_FLAG_UNATTACH = 0x1 << 2;
constexpr auto DEBUG_FLAG_TARGET_ALIVE = 0x1 << 3;
constexpr auto DEBUG_FLAG_READ = 0x1 << 10;
constexpr auto DEBUG_FLAG_WRITE = 0x1 << 11;

constexpr auto DEBUG_FLAGS = (
	0x0
	| DEBUG_FLAG_DEV_CTRL
	| DEBUG_FLAG_ATTACH
	| DEBUG_FLAG_UNATTACH
	| DEBUG_FLAG_TARGET_ALIVE
	| DEBUG_FLAG_READ
	| DEBUG_FLAG_WRITE
);

constexpr auto DEBUG_DEV_CTRL = DEBUG_FLAGS & DEBUG_FLAG_DEV_CTRL;
constexpr auto DEBUG_ATTACH = DEBUG_FLAGS & DEBUG_FLAG_ATTACH;
constexpr auto DEBUG_UNATTACH = DEBUG_FLAGS & DEBUG_FLAG_UNATTACH;
constexpr auto DEBUG_TARGET_ALIVE = DEBUG_FLAGS & DEBUG_FLAG_TARGET_ALIVE;
constexpr auto DEBUG_READ = DEBUG_FLAGS & DEBUG_FLAG_READ;
constexpr auto DEBUG_WRITE = DEBUG_FLAGS & DEBUG_FLAG_WRITE;

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction);

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
}

void debug_print(PCSTR format, ...)
{
#ifndef DEBUG
	UNREFERENCED_PARAMETER(format);
#else
	va_list args;
	va_start(args, format);

	char buffer[1024];

	const auto ret = RtlStringCbVPrintfA(buffer, sizeof(buffer), format, args);

	if (NT_SUCCESS(ret))
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, buffer));
	else
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Error in debug_print: formatting failed\n"));

	va_end(args);

#endif // DEBUG
}

namespace driver {

	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}

	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}

	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		debug_print("[+] Device control called.\n");

		NTSTATUS status = STATUS_UNSUCCESSFUL;

		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		auto request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr) {
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		static PEPROCESS target_process = nullptr;

		const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;

		switch (control_code)
		{
			case codes::attach:
			{
				const auto target_pid = request->process_id;
				debug_print("[+] Attaching to process (%d).\n", target_pid);

				status = PsLookupProcessByProcessId(target_pid, &target_process);
				break;
			}
			case codes::read:
			{
				if (target_process != nullptr)
				{
					const auto target = request->target;
					const auto size = request->size;
					const auto buffer = request->buffer;
					debug_print("[+] Reading %d bytes from %p to %p.\n", size, target, buffer);

					status = MmCopyVirtualMemory(target_process, target,
						PsGetCurrentProcess(), buffer,
						size, KernelMode, &request->return_size);
				}
				else
					debug_print("[-] Tried to read without being attached to a process.\n");
				break;
			}
			case codes::write:
			{
				if (target_process != nullptr)
				{
					const auto target = request->target;
					const auto size = request->size;
					const auto buffer = request->buffer;
					debug_print("[+] Writing %d bytes to %p from %p.\n", size, target, buffer);

					status = MmCopyVirtualMemory(PsGetCurrentProcess(), buffer,
						target_process, target,
						size, KernelMode, &request->return_size);
				}
				else
					debug_print("[-] Tried to write without being attached to a process.\n");
				break;
			}
			default:
				break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(Request);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\AssPileDriver");

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN,
										FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (!NT_SUCCESS(status))
	{
		debug_print("[-] Failed to create driver device.\n");
		return status;
	}

	debug_print("[+] Driver device successfully created.\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\AssPileDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status)) {
		debug_print("[-] Failed to establish symbolic link\n");
		return status;
	}

	debug_print("[+] Driver symbolic link successfully established.\n");
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	debug_print("[+] Driver initialized successfully.\n");

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry() {
	debug_print("[+] DriverEntry called. Initializing driver...\n");

	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\AssPileDriver");

	return IoCreateDriver(&driver_name, &driver_main);
}
