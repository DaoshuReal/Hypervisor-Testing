#include <Includes.h>

volatile LONG Hypervisor::stopThreads = 0;
HANDLE Hypervisor::threadHandles[64];

extern "C" void GuestEntryPoint();
extern "C" void AsmVmrun(UINT64 vmcbPhysicalAddress);

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
    DbgPrint("[CPU %lu] Affinity set to CPU %lu\n", cpuNumber, cpu);

    // Enable SVM
    SVMUtilities::EnableSvmOnCore();
    DbgPrint("[CPU %lu] SVM enabled\n", cpuNumber);

    // Allocate and setup VMCB
    VMCB* vmcb = VMCBUtilities::AllocateVmcb();
    if (!vmcb)
    {
        DbgPrint("[CPU %lu] Failed to allocate VMCB\n", cpuNumber);
        KeRevertToUserAffinityThread();
        PsTerminateSystemThread(STATUS_INSUFFICIENT_RESOURCES);
        return;
    }

    VMCBUtilities::SetupVMCB(vmcb);

    PHYSICAL_ADDRESS highestAddr;
    highestAddr.QuadPart = 0xFFFFFFFFFFFFFFFFULL;

    // Allocate a safe guest stack
    void* guestStack = MmAllocateContiguousMemory(0x1000, highestAddr);
    if (!guestStack)
    {
        DbgPrint("[CPU %lu] Failed to allocate guest stack\n", cpuNumber);
        VMCBUtilities::FreeVmcb(vmcb);
        KeRevertToUserAffinityThread();
        PsTerminateSystemThread(STATUS_INSUFFICIENT_RESOURCES);
        return;
    }
    RtlZeroMemory(guestStack, 0x1000);

    // Setup guest state
    vmcb->state_save_area.rip = (UINT64)&GuestEntryPoint;
    vmcb->state_save_area.rsp = (UINT64)guestStack + 0x1000; // stack grows down
    vmcb->state_save_area.rflags = 0x2; // minimal reserved bit

    // Intercept #PF as an example (safe dummy intercept)
    //vmcb->control_area.intercept_instr1 = (1 << 24);

    UINT64 vmcbPhys = MmGetPhysicalAddress(vmcb).QuadPart;

    DbgPrint("[CPU %lu] Entering guest loop\n", cpuNumber);

    while (InterlockedCompareExchange(&stopThreads, 0, 0) == 0) {
        AsmVmrun(vmcbPhys);

        // Guest has exited. Throttle the host loop
        LARGE_INTEGER delay;
        delay.QuadPart = -5000 * 10; // 50,000ns = 5 microseconds (safe short pause)
        KeDelayExecutionThread(KernelMode, FALSE, &delay);

        DbgPrint("[CPU %lu] VMEXIT at RIP: 0x%llx\n", cpuNumber, vmcb->state_save_area.rip);
    }

    // Free resources
    MmFreeContiguousMemory(guestStack);
    VMCBUtilities::FreeVmcb(vmcb);

    DbgPrint("[CPU %lu] Hypervisor thread exiting\n", cpuNumber);
    KeRevertToUserAffinityThread();
    PsTerminateSystemThread(STATUS_SUCCESS);
}
