#pragma once

#include "driver.h"

#include <ntifs.h>
#include <ntddk.h>

#include "kdebug.h"

#include "../process/process.h"

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction);

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
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

	NTSTATUS set_debug_flags(const size_t new_flags, size_t& old_flags) {
		if (DEBUG_SET_DEBUG_FLAGS)
			debug("[+] Setting debug flags from %p to %p.\n", DEBUG_FLAGS, new_flags);

		old_flags = DEBUG_FLAGS;

		DEBUG_FLAGS = new_flags;

		return STATUS_SUCCESS;
	}

	NTSTATUS unattach(Process& process) {
		if (DEBUG_UNATTACH)
			debug("[+] Unattaching from process.\n");

		if (!process.Connected())
		{
			if (DEBUG_UNATTACH)
				debug("[-] Not attached to a process.\n");
			return STATUS_INVALID_PARAMETER;
		}

		process.Clear();

		if (DEBUG_UNATTACH)
			debug("[+] Unattached from process.\n");

		return STATUS_SUCCESS;
	}

	NTSTATUS attach(Process& process, HANDLE pid) {
		if (process.Connected())
			unattach(process);

		if (DEBUG_ATTACH)
			debug("[+] Attaching to process (%d).\n", pid);

		PEPROCESS target_process = nullptr;

		NTSTATUS status = PsLookupProcessByProcessId(pid, &target_process);
		process.Connect(target_process);

		if (DEBUG_ATTACH)
		{
			if (NT_SUCCESS(status))
				debug("[+] Attached to process (%d).\n", pid);
			else
				debug("[-] Failed to attach to process (%d).\n", pid);
		}

		return status;
	}

	NTSTATUS attach(Process& process, const PWCHAR process_name) {
		if (process.Connected())
			unattach(process);

		if (DEBUG_ATTACH)
			debug("[+] Looking up process: %ws\n", process_name);

		HANDLE pid = 0;

		NTSTATUS status = findProcessIdByName(process_name, pid);

		if (!NT_SUCCESS(status))
		{
			if (DEBUG_ATTACH)
				debug("[-] Failed to find process: %ws\n", process_name);
			return status;
		}

		return attach(process, pid);
	}

	NTSTATUS process_is_alive(Process& process, SIZE_T& is_alive) {
		if (DEBUG_TARGET_ALIVE)
			debug("[+] Checking if target process is alive.\n");

		if (!process.Connected())
		{
			if (DEBUG_TARGET_ALIVE)
				debug("[-] Not attached to a process.\n");

			is_alive = FALSE;

			return STATUS_INVALID_PARAMETER;
		}

		is_alive = process.is_alive();

		if (DEBUG_TARGET_ALIVE)
			debug("[+] Target process is %s.\n", is_alive ? "alive" : "dead");

		return STATUS_SUCCESS;
	}

	NTSTATUS read_memory(Process& process_struct, PVOID destination, PVOID source, SIZE_T size, PSIZE_T return_size) {
		if (DEBUG_READ)
			debug("[+] Reading %d bytes from %p to %p.\n", size, source, destination);

		if (!process_struct.Connected())
		{
			if (DEBUG_READ)
				debug("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(process_struct.eprocess, source, PsGetCurrentProcess(),
			destination, size, KernelMode, return_size);

		if (DEBUG_READ) if (!NT_SUCCESS(status))
			debug("[-] Failed to read memory.\n");

		return status;
	}

	NTSTATUS write_memory(Process& process_struct, PVOID destination, PVOID source, SIZE_T size, PSIZE_T return_size) {
		if (DEBUG_WRITE)
			debug("[+] Writing %d bytes from %p to %p.\n", size, source, destination);

		if (!process_struct.Connected())
		{
			if (DEBUG_READ)
				debug("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(PsGetCurrentProcess(), source, process_struct.eprocess,
			destination, size, KernelMode, return_size);

		if (DEBUG_WRITE) if (!NT_SUCCESS(status))
			debug("[-] Failed to write memory.\n");

		return status;
	}

	NTSTATUS get_module(Process& process, PVOID module_name, uintptr_t& address) {
		return findModuleBaseAddress(process, reinterpret_cast<PWCHAR>(module_name), address);
	}


	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		// This holds the process across calls to the driver.
		static Process target_process{};

		if (DEBUG_DEV_CTRL)
			debug("[+] Device control called.\n");

		// Get request packet from irp
		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		Request* request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr)
		{
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;
		}

		// --------------------------------------------------------------------------------
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		const auto control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;

		switch (control_code)
		{
		case codes::nothing:
			status = STATUS_SUCCESS;
			break;
		case codes::attach_pid:
			status = attach(target_process, request->process_id);
			break;
		case codes::attach_name:
			status = attach(target_process, (PWCHAR)request->buffer);
			break;
		case codes::unattach:
			status = unattach(target_process);
			break;
		case codes::target_is_alive:
			status = process_is_alive(target_process, request->return_value);
			break;
		case codes::read:
			status = read_memory(target_process, request->buffer, request->target_address, request->size, &request->return_size);
			break;
		case codes::write:
			status = write_memory(target_process, request->target_address, request->buffer, request->size, &request->return_size);
			break;
		case codes::get_module:
			status = get_module(target_process, request->buffer, request->return_value);
			break;
		case codes::set_debug_flags:
			status = set_debug_flags(*(size_t*)request->buffer, request->return_value);
			break;
		default:
			debug("[-] Unknown control code: %x\n", control_code);
			break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(Request);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\AssPileDriver");

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (!NT_SUCCESS(status))
	{
		debug("[-] Failed to create driver device.\n");
		return status;
	}

	debug("[+] Driver device successfully created.\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\AssPileDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status))
	{
		debug("[-] Failed to establish symbolic link\n");
		return status;
	}

	debug("[+] Driver symbolic link successfully established.\n");
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	debug("[+] Driver initialized successfully.\n");

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry() {
	debug("[+] DriverEntry called. Initializing driver...\n");

	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\AssPileDriver");

	return IoCreateDriver(&driver_name, &driver_main);
}
