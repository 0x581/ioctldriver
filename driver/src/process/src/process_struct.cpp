#pragma once

#include "../includes/process_struct.h"

#include "../includes/get_info.h"

#include "kdebug.h"


Process::Process() :
	eprocess(nullptr),
	pid(nullptr),
	cr3{ 0 },
	exit_status{ nullptr },
	process_section_base_address{ 0 },
	pPeb{ nullptr }
{}

bool Process::Attached() const
{
	return eprocess != nullptr;
}

void Process::Connect(PEPROCESS target_process)
{
	if (this->Attached())
		this->Clear();

	if (DEBUG_PROCESS_STRUCT)
		debug("[+] Connecting to process.\n");

	if (target_process == nullptr)
	{
		if (DEBUG_PROCESS_STRUCT)
			debug("[-] Target process is null.\n");
		return;
	}

	const PLONG p_exit_status = processGetExitStatusAddress(target_process);
	if (p_exit_status == nullptr || *p_exit_status != STATUS_PENDING)
	{
		if (DEBUG_PROCESS_STRUCT)
			debug("[-] Target process is closing.\n");
		return;
	}


	this->eprocess = target_process;
	this->pid = processGetPid(this->eprocess);

	this->cr3 = processGetCr3(this->eprocess);
	this->exit_status = p_exit_status;

	this->process_section_base_address = processGetBaseAddress(this->eprocess);
	this->pPeb = processGetPebPtr(this->eprocess);

	if (DEBUG_PROCESS_STRUCT)
		debug(
			"[+] Connected:\n"
			"[+]     PID: %d\n"
			"[+]     CR3: 0x%p\n"
			"[+]     Exit status: 0x%p\n"
			"[+]     PEB: 0x%p\n"
			"[+]     Process section base address: 0x%p\n",
			this->pid,
			this->cr3,
			this->exit_status,
			this->pPeb,
			this->process_section_base_address
		);
}

void Process::Clear()
{
	if (DEBUG_DETACH)
		debug("[+] Clearing process data.\n");

	if (this->eprocess)
	{
		ObDereferenceObject(eprocess);
	}
	this->eprocess = nullptr;

	this->pid = nullptr;
	this->cr3 = 0;
	this->exit_status = nullptr;
	this->pPeb = nullptr;
	this->process_section_base_address = 0;
}

bool Process::is_alive() const
{
	if (!this->Attached())
		return false;

	return (*this->exit_status == STATUS_PENDING);
}
