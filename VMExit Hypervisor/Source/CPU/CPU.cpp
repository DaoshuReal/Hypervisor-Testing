#include <Includes.h>

ULONG CPUUtilities::numCpus = 0;

void CPUUtilities::GetCpuId(int leaf, int subLeaf, int CpuInfo[4])
{
	__cpuidex(CpuInfo, leaf, subLeaf);
}

uint64_t CPUUtilities::ReadMSR(uint32_t msr)
{
	return __readmsr(msr);
}

bool CPUUtilities::HasFeature(int featureBit, int leaf, int reg)
{
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, leaf);

    if (reg < 0 || reg > 3)
        return false;

    return (cpuInfo[reg] & (1 << featureBit)) != 0;
}

ULONG CPUUtilities::GetNumCpus()
{
    if (numCpus == 0)
        numCpus = KeQueryActiveProcessorCount(NULL);

    return numCpus;
}