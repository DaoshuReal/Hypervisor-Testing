#pragma once

#include <ntifs.h>
#include <cstdint>

class CPUUtilities
{
public:
	static void GetCpuId(int leaf, int subLeaf, int CpuInfo[4]);
	static uint64_t ReadMSR(uint32_t msr);
	static bool HasFeature(int featureBit, int leaf = 1, int reg = 3);
	static ULONG GetNumCpus();

private:
	static ULONG numCpus;
};