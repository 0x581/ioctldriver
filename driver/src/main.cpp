#pragma once

#include "debug.h"
#include "driver.h"

#include <basetsd.h>
#include <ntddk.h>
#include <ntdef.h>
#include <ntifs.h>
#include <ntstatus.h>
#include <wdm.h>

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

	NTSTATUS unattach(PEPROCESS* target_process) {
		if (DEBUG_UNATTACH)
			debug_print("[+] Unattaching from process.\n");

		if (!*target_process) {
			if (DEBUG_UNATTACH)
				debug_print("[-] Not attached to a process.\n");
			return STATUS_INVALID_PARAMETER;
		}

		ObDereferenceObject(*target_process);
		*target_process = nullptr;

		if (DEBUG_UNATTACH)
			debug_print("[+] Unattached from process.\n");

		return STATUS_SUCCESS;
	}

	NTSTATUS target_is_alive(PEPROCESS target_process, PSIZE_T is_alive) {
		if (DEBUG_TARGET_ALIVE)
			debug_print("[+] Checking if target process is alive.\n");

		if (!target_process) {
			if (DEBUG_TARGET_ALIVE)
				debug_print("[-] Not attached to a process.\n");
			return STATUS_INVALID_PARAMETER;
		}

		NTSTATUS status = STATUS_SUCCESS;

		// Check if the process is alive
		//if (KeReadStateProcess(target_process)) {
		//	if (DEBUG_TARGET_ALIVE)
		//		debug_print("[-] Target process is dead.\n");
		//	return STATUS_INVALID_PARAMETER;
		//}

		return status;
	}

	NTSTATUS attach(HANDLE pid, PEPROCESS* target_process) {
		if (*target_process)
			unattach(target_process);

		if (DEBUG_ATTACH)
			debug_print("[+] Attaching to process (%d).\n", pid);

		NTSTATUS status = PsLookupProcessByProcessId(pid, target_process);

		if (DEBUG_ATTACH)
		{
			if (!NT_SUCCESS(status))
				debug_print("[-] Failed to attach to process (%d).\n", pid);
			else
				debug_print("[+] Attached to process (%d).\n", pid);
		}

		return status;
	}

	NTSTATUS read_memory(PVOID destination, PVOID source, SIZE_T size, PEPROCESS src_process, PSIZE_T return_size) {
		if (DEBUG_READ)
			debug_print("[+] Reading %d bytes from %p to %p.\n", size, source, destination);

		if (!src_process) {
			if (DEBUG_READ)
				debug_print("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(src_process, source, PsGetCurrentProcess(),
												destination, size, KernelMode, return_size);

		if (DEBUG_READ && !NT_SUCCESS(status))
			debug_print("[-] Failed to read memory.\n");

		return status;
	}

	NTSTATUS write_memory(PVOID destination, PVOID source, SIZE_T size, PEPROCESS dest_process, PSIZE_T return_size) {
		if (DEBUG_WRITE)
			debug_print("[+] Writing %d bytes from %p to %p.\n", size, source, destination);

		if (!dest_process) {
			if (DEBUG_READ)
				debug_print("[-] Not attached to a procss.\n");

			return STATUS_INVALID_PARAMETER;
		}

		const auto status = MmCopyVirtualMemory(PsGetCurrentProcess(), source, dest_process,
			destination, size, KernelMode, return_size);

		if (DEBUG_WRITE && !NT_SUCCESS(status))
			debug_print("[-] Failed to write memory.\n");

		return status;
	}


	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);

		// This holds the process across calls to the driver.
		static PEPROCESS target_process = nullptr;

		if (DEBUG_DEV_CTRL)
			debug_print("[+] Device control called.\n");

		// Get request packet from irp
		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		Request *request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr) {
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;
		}

		// --------------------------------------------------------------------------------
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		const auto control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;

		switch (control_code)
		{
			case codes::attach:
				status = attach(request->process_id, &target_process);
				break;
			case codes::unattach:
				status = unattach(&target_process);
				break;
			case codes::target_is_alive:
				status = target_is_alive(target_process, &(request->return_value));
				break;
			case codes::read:
				status = read_memory(request->buffer, request->target, request->size,
									 target_process, &request->return_size);
				break;
			case codes::write:
				status = write_memory(request->target, request->buffer, request->size,
									  target_process, &request->return_size);
				break;
			default:
				if (DEBUG_DEV_CTRL)
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
