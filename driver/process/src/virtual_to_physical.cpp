#pragma once

#include <ntifs.h>

#include "../includes/page_definitions.h"
#include "../includes/get_info.h"

#include "../../kdebug/kdebug.h"


NTSTATUS ReadPhysicalAddress(uintptr_t TargetAddress, PVOID lpBuffer, size_t Size, size_t* BytesRead) {
	MM_COPY_ADDRESS AddrToRead = { 0 };
	AddrToRead.PhysicalAddress.QuadPart = TargetAddress;

	return MmCopyMemory(lpBuffer, AddrToRead, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}

//MmMapIoSpaceEx limit is page 4096 byte
NTSTATUS WritePhysicalAddress(uintptr_t TargetAddress, PVOID lpBuffer, size_t Size, size_t* BytesWritten) {
	if (!TargetAddress)
		return STATUS_INVALID_PARAMETER;

	PHYSICAL_ADDRESS AddrToWrite = { 0 };
	AddrToWrite.QuadPart = TargetAddress;

	PVOID pmapped_mem = MmMapIoSpaceEx(AddrToWrite, Size, PAGE_READWRITE);

	if (!pmapped_mem)
		return STATUS_UNSUCCESSFUL;

	memcpy(pmapped_mem, lpBuffer, Size);

	*BytesWritten = Size;
	MmUnmapIoSpace(pmapped_mem, Size);
	return STATUS_SUCCESS;
}

uintptr_t TranslateLinearAddress(uintptr_t pml4, VirtualAddress vAddr) {
	if (pml4 & (PAGE_SIZE - 1))
	{
		debug("[-] PML4 is not aligned to page size.\n");
		return 0;
	}

	const uintptr_t pml4_idx = vAddr.get_pml4_index();
	const uintptr_t pml3_idx = vAddr.get_pml3_index();
	const uintptr_t pml2_idx = vAddr.get_pml2_index();
	const uintptr_t pml1_idx = vAddr.get_pml1_index();

	size_t readsize = 0;

	PageMapLevel4Entry pml4e{ };
	ReadPhysicalAddress(pml4 + PAGE_ENTRY_SIZE * pml4_idx, &pml4e, sizeof(pml4e), &readsize);
	if (!pml4e.present())
		return 0;

	PageMapLevel3Entry pml3e{ };
	ReadPhysicalAddress(pml4e.get_address() + PAGE_ENTRY_SIZE * pml3_idx, &pml3e, sizeof(pml3e), &readsize);
	if (!pml3e.present())
		return 0;

	/* 1GB huge page */
	if (pml3e.is_huge())
		return pml3e.get_huge_address() | vAddr.get_offset_1g();

	PageMapLevel2Entry pml2e{ };
	ReadPhysicalAddress(pml3e.get_address() + PAGE_ENTRY_SIZE * pml2_idx, &pml2e, sizeof(pml2e), &readsize);
	if (!pml2e.present())
		return 0;

	/* 2MB large page */
	if (pml2e.is_large())
		return (pml2e.get_large_address() << 21) + vAddr.get_offset_2m();

	PageMapLevel1Entry pml1e{ };
	ReadPhysicalAddress(pml2e.get_address() + PAGE_ENTRY_SIZE * pml1_idx, &pml1e, sizeof(pml1e), &readsize);
	if (!pml1e.present())
		return 0;

	return pml1e.get_address() + vAddr.get_offset();
}

NTSTATUS ReadVirtual(uintptr_t dirbase, uintptr_t address, PVOID buffer, size_t size, size_t* read) {
	uintptr_t pAddress = TranslateLinearAddress(dirbase, address);

	return ReadPhysicalAddress(pAddress, buffer, size, read);
}

NTSTATUS WriteVirtual(uintptr_t dirbase, uintptr_t address, PVOID buffer, size_t size, size_t* written) {
	uintptr_t pAddress = TranslateLinearAddress(dirbase, address);

	return WritePhysicalAddress(pAddress, buffer, size, written);
}

//
NTSTATUS ReadProcessMemory(PEPROCESS pProcess, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* read) {
	uintptr_t process_dirbase = getProcessCr3(pProcess);

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	size_t bytes_read = 0;
	size_t remaining_size = size;

	while (remaining_size)
	{
		uintptr_t CurPhysAddr = TranslateLinearAddress(process_dirbase, (uintptr_t)Address + bytes_read);

		if (!CurPhysAddr)
			return STATUS_UNSUCCESSFUL;

		size_t ReadSize = min(PAGE_SIZE - (CurPhysAddr & 0xFFF), remaining_size);
		size_t BytesRead = 0;

		status = ReadPhysicalAddress(CurPhysAddr, (PVOID)((ULONG64)AllocatedBuffer + bytes_read), ReadSize, &BytesRead);

		remaining_size -= BytesRead;
		bytes_read += BytesRead;

		if (status != STATUS_SUCCESS)
			break;

		if (BytesRead == 0)
			break;
	}

	*read = bytes_read;
	return status;
}

NTSTATUS WriteProcessMemory(PEPROCESS process, PVOID Address, PVOID AllocatedBuffer, size_t size, size_t* written) {
	uintptr_t process_dirbase = getProcessCr3(process);

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	size_t bytes_written = 0;
	size_t remaining_size = size;
	while (remaining_size)
	{
		uintptr_t CurPhysAddr = TranslateLinearAddress(process_dirbase, (ULONG64)Address + bytes_written);

		if (!CurPhysAddr)
			return STATUS_UNSUCCESSFUL;

		size_t WriteSize = min(PAGE_SIZE - (CurPhysAddr & 0xFFF), remaining_size);
		size_t BytesWritten = 0;

		status = WritePhysicalAddress(CurPhysAddr, (PVOID)((uintptr_t)AllocatedBuffer + bytes_written), WriteSize, &BytesWritten);

		remaining_size -= BytesWritten;
		bytes_written += BytesWritten;

		if (status != STATUS_SUCCESS)
			break;

		if (BytesWritten == 0)
			break;
	}

	*written = bytes_written;
	return status;
}