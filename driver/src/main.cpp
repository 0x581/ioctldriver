#pragma once

#include "debug.h"
#include "driver.h"

#include <ntifs.h>
#include <ntddk.h>

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction);

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
}

namespace driver {
	struct OpenProcess {
		PEPROCESS process;
		HANDLE pid;

		BOOLEAN is_alive;

		OpenProcess() : process(nullptr), pid(nullptr), is_alive(FALSE) {}

		BOOLEAN Connected() const { return process != nullptr; }

		VOID Connect(PEPROCESS target_process) {
			if (this->Connected())
				this->Clear();

			this->process = target_process;
			this->pid = PsGetProcessId(target_process);
			this->is_alive = TRUE;
		}

		VOID Clear() {
			if (this->process)
			{
				ObDereferenceObject(process);
				this->process = nullptr;
			}
			this->pid = nullptr;
			this->is_alive = FALSE;
		}
	};

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

	NTSTATUS set_debug_level(const size_t new_flags, size_t& old_flags) {
		if (DEBUG_SET_DEBUG_LEVEL)
			debug_print("[+] Setting debug level from %p to %p.\n", DEBUG_FLAGS, new_flags);

		old_flags = DEBUG_FLAGS;

		DEBUG_FLAGS = new_flags;

		return STATUS_SUCCESS;
	}

	NTSTATUS unattach(OpenProcess& process_struct) {
		if (DEBUG_UNATTACH)
			debug_print("[+] Unattaching from process.\n");

		if (!process_struct.process)
		{
			if (DEBUG_UNATTACH)
				debug_print("[-] Not attached to a process.\n");
			return STATUS_INVALID_PARAMETER;
		}

		process_struct.Clear();

		if (DEBUG_UNATTACH)
			debug_print("[+] Unattached from process.\n");

		return STATUS_SUCCESS;
	}

	NTSTATUS attach(OpenProcess& process_struct, HANDLE pid) {
		if (process_struct.Connected())
			unattach(process_struct);

		if (DEBUG_ATTACH)
			debug_print("[+] Attaching to process (%d).\n", pid);

		NTSTATUS status = PsLookupProcessByProcessId(pid, &process_struct.process);

		if (DEBUG_ATTACH)
		{
			if (!NT_SUCCESS(status))
				debug_print("[-] Failed to attach to process (%d).\n", pid);
			else
				debug_print("[+] Attached to process (%d).\n", pid);
		}

		return status;
	}

	NTSTATUS process_is_alive(OpenProcess& process_struct, SIZE_T& is_alive) {
		if (DEBUG_TARGET_ALIVE)
			debug_print("[+] Checking if target process is alive.\n");

		if (!process_struct.Connected())
		{
			if (DEBUG_TARGET_ALIVE)
				debug_print("[-] Not attached to a process.\n");

			is_alive = FALSE;

			return STATUS_INVALID_PARAMETER;
		}

		//const auto exit_status = PsGetProcessExitStatus(target_process);

		LARGE_INTEGER timeout_0{ 0 };
		const auto exit_status = KeWaitForSingleObject(process_struct.process, KWAIT_REASON::Executive,
			KernelMode, FALSE, &timeout_0);

		is_alive = (exit_status == STATUS_TIMEOUT);

		if (DEBUG_TARGET_ALIVE)
			debug_print("[+] Target process is %s.\n", is_alive ? "alive" : "dead");

		return STATUS_SUCCESS;
	}

	NTSTATUS read_memory(OpenProcess& process_struct, PVOID destination, PVOID source, SIZE_T size, PSIZE_T return_size) {
		if (DEBUG_READ)
			debug_print("[+] Reading %d bytes from %p to %p.\n", size, source, destination);

		if (!process_struct.Connected())
		{
			if (DEBUG_READ)
				debug_print("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(process_struct.process, source, PsGetCurrentProcess(),
			destination, size, KernelMode, return_size);

		if (DEBUG_READ) if (!NT_SUCCESS(status))
			debug_print("[-] Failed to read memory.\n");

		return status;
	}

	NTSTATUS write_memory(OpenProcess& process_struct, PVOID destination, PVOID source, SIZE_T size, PSIZE_T return_size) {
		if (DEBUG_WRITE)
			debug_print("[+] Writing %d bytes from %p to %p.\n", size, source, destination);

		if (!process_struct.Connected())
		{
			if (DEBUG_READ)
				debug_print("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(PsGetCurrentProcess(), source, process_struct.process,
			destination, size, KernelMode, return_size);

		if (DEBUG_WRITE) if (!NT_SUCCESS(status))
			debug_print("[-] Failed to write memory.\n");

		return status;
	}

	NTSTATUS get_module(OpenProcess& process_struct, PVOID module_name, uintptr_t& address) {
		UNREFERENCED_PARAMETER(module_name);
		UNREFERENCED_PARAMETER(process_struct);
		UNREFERENCED_PARAMETER(address);

		// Get the PEB of the Process
		//PEPROCESS peb = PsGetProcessPeb(process);

		return STATUS_NOT_IMPLEMENTED;
	}


	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		// This holds the process across calls to the driver.
		static OpenProcess target_process{};

		if (DEBUG_DEV_CTRL)
			debug_print("[+] Device control called.\n");

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
		case codes::attach:
			status = attach(target_process, request->process_id);
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
		case codes::set_debug_level:
			status = set_debug_level(*(size_t*)request->buffer, request->return_value);
			break;
		default:
			debug_print("[-] Unknown control code: %x\n", control_code);
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
		debug_print("[-] Failed to create driver device.\n");
		return status;
	}

	debug_print("[+] Driver device successfully created.\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\AssPileDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status))
	{
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
