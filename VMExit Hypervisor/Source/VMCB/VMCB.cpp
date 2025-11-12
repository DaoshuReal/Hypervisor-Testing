#include <Includes.h>

VMCB* VMCBUtilities::AllocateVmcb()
{
	PHYSICAL_ADDRESS highest;
	highest.QuadPart = 0xFFFFFFFFFFFFFFFFULL;

	PVOID vmcb = MmAllocateContiguousMemory(VMCB_SIZE, highest);
	if (vmcb)
		RtlZeroMemory(vmcb, VMCB_SIZE);

	return (VMCB*)vmcb;
}

void VMCBUtilities::FreeVmcb(VMCB* vmcb)
{
	if (vmcb)
	{
		MmFreeContiguousMemory(vmcb);
		vmcb = nullptr;
	}
}
