#include <Includes.h>

volatile LONG Hypervisor::stopThreads = 0;
HANDLE Hypervisor::threadHandles[64];

NTSTATUS Hypervisor::StartVmmThreads()
{
	ULONG numCpus = CPUUtilities::GetNumCpus();
	for (ULONG cpu = 0; cpu < numCpus; ++cpu)
	{
		HANDLE currentHandle = NULL;

		NTSTATUS status = PsCreateSystemThread(&currentHandle, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, (PKSTART_ROUTINE)HypervisorThread, (PVOID)(ULONG_PTR)cpu);
		if (!NT_SUCCESS(status))
			return status;

		threadHandles[cpu] = currentHandle;
	}

	return STATUS_SUCCESS;
}

VOID Hypervisor::StopVmmThreads()
{
	InterlockedExchange(&stopThreads, 1);

	ULONG numCpus = CPUUtilities::GetNumCpus();
	for (ULONG cpu = 0; cpu < numCpus; ++cpu)
	{
		HANDLE currentHandle = threadHandles[cpu];
		if (currentHandle)
		{
			ZwWaitForSingleObject(currentHandle, FALSE, nullptr);
			ZwClose(currentHandle);

			threadHandles[cpu] = NULL;
		}
	}
}

VOID Hypervisor::HypervisorThread(PVOID Context)
{
	ULONG cpuNumber = KeGetCurrentProcessorNumber();
	DbgPrint("[CPU %d] Hypervisor thread started\n", cpuNumber);

	ULONG cpu = (ULONG)(ULONG_PTR)Context;

	KAFFINITY affinityMask = (KAFFINITY)1 << cpu;

	KeSetSystemAffinityThread(affinityMask);

	SVMUtilities::EnableSvmOnCore();
	DbgPrint("SVM Enabled On CPU: %lu\n", cpuNumber);

	KeRevertToUserAffinityThread();

	PsTerminateSystemThread(STATUS_SUCCESS);
}