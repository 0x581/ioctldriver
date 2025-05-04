#pragma once

#include "../process.h"

#include "../includes/offsets.h"
#include "../includes/get_info.h"

#include "../../kdebug/kdebug.h"


Process::Process() :
	eprocess(nullptr),
	pid(nullptr),
	cr3{ 0 },
	exit_status{ nullptr },
	process_section_base_address{ 0 } {}

bool Process::Connected() const {
	return eprocess != nullptr;
}

void Process::Connect(PEPROCESS target_process) {
	if (this->Connected())
		this->Clear();

	if (DEBUG_ATTACH)
		debug("[+] Connecting to process.\n");

	this->eprocess = target_process;
	this->pid = PsGetProcessId(target_process);

	this->cr3 = getProcessCr3(this->eprocess);
	this->exit_status = getProcessExitStatusAddress(this->eprocess);

	this->process_section_base_address = getProcessBaseAddress(this->eprocess);

	if (DEBUG_ATTACH)
		debug(
			"[+] Connected.\n"
			"[+]	PID: %d\n"
			"[+]	CR3: %p\n"
			"[+]	Exit status: %p\n"
			"[+]	Process section base address: %p\n",
			this->pid,
			this->cr3,
			this->exit_status,
			this->process_section_base_address
		);
}

void Process::Clear() {
	if (DEBUG_UNATTACH)
		debug("[+] Clearing process data.\n");

	if (this->eprocess)
	{
		ObDereferenceObject(eprocess);
	}
	this->eprocess = nullptr;

	this->pid = nullptr;
	this->cr3 = 0;
	this->exit_status = nullptr;
	this->process_section_base_address = 0;
}

bool Process::is_alive() const {
	if (!this->Connected())
		return false;

	const LONG status = *this->exit_status;

	const auto real_status = PsGetProcessExitStatus(this->eprocess);

	if (real_status != status)
	{
		debug("[!] Fetched wrong status code in is_alive()!");
	}

	return status == STATUS_PENDING;
}