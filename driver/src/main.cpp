#pragma once

#include <ntifs.h>
#include <ntddk.h>

#include "kdebug.h"
#include "event.h"

#include "driver_defines.h"
#include "ioctl_codes.h"
#include "requests.h"

#include "../process/process.h"

#include "driver_functions/driver_functions.h"

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction);
}

#define CHECK_REQUEST_SIZE(request_data, Request, DEBUG_FLAG) \
{ \
	if (request_data.InputBufferLength < sizeof(Request) || request_data.OutputBufferLength < sizeof(Request)) \
	{ \
		if (DEBUG_FLAG) { debug("[-] Invalid request size.\n"); } \
		status = STATUS_INVALID_PARAMETER; \
		request_size = 0; \
		break; \
	} \
	else { request_size = sizeof(Request); } \
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

		// This holds the process across calls to the driver.
		static Process target_process{};

		if (DEBUG_DEV_CTRL)
			debug("[+] Device control called.\n");

		// Get request packet from irp
		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		PVOID request = irp->AssociatedIrp.SystemBuffer;

		if (stack_irp == nullptr || request == nullptr)
		{
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;
		}

		// --------------------------------------------------------------------------------
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		// The value of this variable is set in the CHECK_REQUEST_SIZE macro.
		size_t request_size = 0;

		const auto request_data = stack_irp->Parameters.DeviceIoControl;

		if (DEBUG_DEV_CTRL)
			debug("[+]     Control code: %u\n", request_data.IoControlCode);

		switch (request_data.IoControlCode)
		{
			//
			// Driver state
		case ioctl_codes::verify_connection:
			CHECK_REQUEST_SIZE(request_data, RequestVerifyConnection, DEBUG_DEV_CTRL);
			status = STATUS_SUCCESS;
			break;
		case ioctl_codes::attach:
			CHECK_REQUEST_SIZE(request_data, RequestAttach, DEBUG_ATTACH);
			status = attach(target_process, (RequestAttach*)request);
			break;
		case ioctl_codes::detach:
			CHECK_REQUEST_SIZE(request_data, RequestUnattach, DEBUG_UNATTACH);
			status = detach(target_process);
			break;
			//
			// Target state
		case ioctl_codes::is_process_alive:
			CHECK_REQUEST_SIZE(request_data, RequestIsProcessAlive, DEBUG_TARGET_ALIVE);
			status = is_process_alive(target_process, (RequestIsProcessAlive*)request);
			break;
			//
			// IO
		case ioctl_codes::read_memory:
			CHECK_REQUEST_SIZE(request_data, RequestReadMemory, DEBUG_READ);
			status = read_memory(target_process, (RequestReadMemory*)request);
			break;
		case ioctl_codes::write_memory:
			CHECK_REQUEST_SIZE(request_data, RequestWriteMemory, DEBUG_WRITE);
			status = write_memory(target_process, (RequestWriteMemory*)request);
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
			CHECK_REQUEST_SIZE(request_data, RequestGetModuleBase, DEBUG_GET_MODULE);
			status = get_module_base(target_process, (RequestGetModuleBase*)request);
			break;
		case ioctl_codes::enumerate_modules:
			debug("[-] Module enumeration not implemented.\n");
			status = STATUS_NOT_IMPLEMENTED;
			break;
		case ioctl_codes::set_debug_flags:
			CHECK_REQUEST_SIZE(request_data, RequestSetDebugFlags, DEBUG_SET_DEBUG_FLAGS);
			status = set_debug_flags((RequestSetDebugFlags*)request);
			break;
		default:
			debug("[-] Unknown control code: %x\n", request_data.IoControlCode);
			break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = request_size;

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, device_name_w);

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, DRIVER_FILE_DEVICE,
		FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (!NT_SUCCESS(status))
	{
		WriteToEventLog(
			driver_object, 0xC0000001, status, L"Failed to create device object.");
		debug("[-] Failed to create driver device.\n");
		return status;
	}

	debug("[+] Driver device successfully created.\n");

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
