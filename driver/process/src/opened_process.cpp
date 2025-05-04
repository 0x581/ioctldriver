#pragma once

#include "../process.h"

#include "../includes/offsets.h"
#include "../includes/get_info.h"

#include "../../kdebug/kdebug.h"


OpenedProcess::OpenedProcess() :
	process(nullptr),
	pid(nullptr),
	cr3{ 0 },
	exit_status{ nullptr } {}

bool OpenedProcess::Connected() const {
	return process != nullptr;
}

void OpenedProcess::Connect(PEPROCESS target_process) {
	if (this->Connected())
		this->Clear();

	this->process = target_process;
	this->pid = PsGetProcessId(target_process);

	this->cr3 = GetProcessCr3(this->process);
	this->exit_status = (PLONG)((char*)this->process + getProcessExitStatusOffset());

	this->process_section_base_address = GetProcessBaseAddress(this->process);
}

void OpenedProcess::Clear() {
	if (this->process)
	{
		ObDereferenceObject(process);
		this->process = nullptr;
	}

	this->pid = nullptr;
	this->exit_status = nullptr;
	this->cr3 = 0;
}

bool OpenedProcess::is_alive() const {
	if (!this->Connected())
		return false;

	const LONG status = *this->exit_status;

	const auto real_status = PsGetProcessExitStatus(this->process);

	if (real_status != status)
	{
		debug("[!] Fetched wrong status code!");
	}

	return status == STATUS_PENDING;
}