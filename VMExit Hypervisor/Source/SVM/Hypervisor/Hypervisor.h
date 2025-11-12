#pragma once

#include <ntifs.h>

class Hypervisor
{
public:
	static NTSTATUS StartVmmThreads();
	static VOID StopVmmThreads();
	static VOID HypervisorThread(PVOID Context);

private:
	static volatile LONG stopThreads;
	static HANDLE threadHandles[64];
};