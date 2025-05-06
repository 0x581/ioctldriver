#pragma once

#include <ntifs.h>
#include <ntddk.h>

#include "kdebug.h"
#include "event.h"

#include "ioctl_codes.h"
#include "requests.h"

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

	NTSTATUS set_debug_flags(Request* request) {
		request->return_value = DEBUG_FLAGS.raw;

		DEBUG_FLAGS.raw = request->value;

		if (DEBUG_SET_DEBUG_FLAGS)
			debug("[+] Debug flags updated from %p to %p.\n", request->return_value, DEBUG_FLAGS.raw);

		return STATUS_SUCCESS;
	}

	NTSTATUS unattach(Process& process) {
		if (DEBUG_UNATTACH)
			debug("[+] Unattaching from process.\n");

		if (!process.Attached())
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

	NTSTATUS attach(Process& process, Request* request) {
		if (process.Attached())
			unattach(process);

		const PWCHAR process_name = static_cast<PWCHAR>(request->buffer);

		if (DEBUG_ATTACH)
			debug("[+] Attaching to process: %ws\n", process_name);

		PEPROCESS target_process = nullptr;
		NTSTATUS status = findProcessByName(process_name, target_process);

		if (NT_SUCCESS(status))
			process.Connect(target_process);

		request->pid = process.pid;

		if (DEBUG_ATTACH)
		{
			if (NT_SUCCESS(status))
				debug("[+] Attached to process: %ws (PID %d)\n", process_name, process.pid);
			else
				debug("[-] Failed to attach to process: %ws\n", process_name);
		}

		return status;
	}

	NTSTATUS process_is_alive(Process& process, Request* request) {
		if (DEBUG_TARGET_ALIVE)
			debug("[+] Checking if target process is alive.\n");

		if (!process.Attached())
		{
			if (DEBUG_TARGET_ALIVE)
				debug("[-] Not attached to a process.\n");

			request->return_value = false;

			return STATUS_INVALID_PARAMETER;
		}

		request->return_value = process.is_alive();

		if (DEBUG_TARGET_ALIVE)
			debug("[+] Target process is %s.\n", request->return_value ? "alive" : "dead");

		return STATUS_SUCCESS;
	}

	NTSTATUS read_memory(Process& process, Request* request) {
		PVOID destination = request->buffer;
		PVOID source = request->address;
		SIZE_T size = request->size;
		PSIZE_T return_size = &request->return_size;

		if (DEBUG_READ)
			debug("[+] Reading %d bytes from %p to %p.\n", size, source, destination);

		if (!process.Attached())
		{
			if (DEBUG_READ)
				debug("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(process.eprocess, source, PsGetCurrentProcess(),
			destination, size, KernelMode, return_size);

		if (DEBUG_READ) if (!NT_SUCCESS(status))
			debug("[-] Failed to read memory.\n");

		return status;
	}

	NTSTATUS write_memory(Process& process, Request* request) {
		PVOID destination = request->address;
		PVOID source = request->buffer;
		SIZE_T size = request->size;
		PSIZE_T return_size = &request->return_size;

		if (DEBUG_WRITE)
			debug("[+] Writing %d bytes from %p to %p.\n", size, source, destination);

		if (!process.Attached())
		{
			if (DEBUG_READ)
				debug("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(PsGetCurrentProcess(), source, process.eprocess,
			destination, size, KernelMode, return_size);

		if (DEBUG_WRITE) if (!NT_SUCCESS(status))
			debug("[-] Failed to write memory.\n");

		return status;
	}

	NTSTATUS get_module(Process& process, Request* request) {
		PVOID module_name = request->buffer;
		uintptr_t& address = request->return_value;;

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

		if (DEBUG_DEV_CTRL)
			debug("[+]     Control code: %u\n", control_code);

		switch (control_code)
		{
			//
			// Driver state
		case ioctl_codes::verify_connection:
			status = STATUS_SUCCESS;
			break;
		case ioctl_codes::attach:
			status = attach(target_process, request);
			break;
		case ioctl_codes::unattach:
			status = unattach(target_process);
			break;
			//
			// Target state
		case ioctl_codes::is_target_alive:
			status = process_is_alive(target_process, request);
			break;
			//
			// IO
		case ioctl_codes::read_memory:
			status = read_memory(target_process, request);
			break;
		case ioctl_codes::write_memory:
			status = write_memory(target_process, request);
			break;
			//
			// Memory
		case ioctl_codes::allocate_memory:
		case ioctl_codes::free_memory:
		case ioctl_codes::protect_memory:
		case ioctl_codes::query_memory:
		case ioctl_codes::scan_memory:
			debug("[-] Memory operations not implemented.\n");
			status = STATUS_NOT_IMPLEMENTED;
			break;
			//
			// Threads
		case ioctl_codes::create_thread:
		case ioctl_codes::remove_thread:
			debug("[-] Thread operations not implemented.\n");
			status = STATUS_NOT_IMPLEMENTED;
			break;
			//
			// Modules
		case ioctl_codes::get_module_base:
			status = get_module(target_process, request);
			break;
		case ioctl_codes::enumerate_modules:
			debug("[-] Module enumeration not implemented.\n");
			status = STATUS_NOT_IMPLEMENTED;
			break;
		case ioctl_codes::set_debug_flags:
			status = set_debug_flags(request);
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
		WriteToEventLog(
			driver_object, 0xC0000001, status, L"Failed to create device object.");
		debug("[-] Failed to create driver device.\n");
		return status;
	}

	debug("[+] Driver device successfully created.\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\AssPileDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status))
	{
		WriteToEventLog(
			driver_object, 0xC0000002, status, L"Failed to create symbolic link.");
		debug("[-] Failed to establish symbolic link\n");

		IoDeleteDevice(device_object);

		return status;
	}

	debug("[+] Driver symbolic link successfully established.\n");
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	WriteToEventLog(
		driver_object, 0xC0000003, STATUS_SUCCESS, L"Driver initialized successfully.");

	debug("[+] Driver initialized successfully.\n");

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry() {
	debug("[+] DriverEntry called. Initializing driver...\n");

	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\AssPileDriver");

	return IoCreateDriver(&driver_name, &driver_main);
}
