#include <Includes.h>

bool SVMUtilities::IsSvmEnabled()
{
    int cpuInfo[4] = { 0 };
    CPUUtilities::GetCpuId(0x80000000, 0, cpuInfo);
    if (cpuInfo[0] < 0x8000000A)
        return false;

    if (!CPUUtilities::HasFeature(0, 0x8000000A, 3))
        return false;

    uint64_t vmcr = CPUUtilities::ReadMSR(0xC0010114);
    if (vmcr & (1ULL << 4))
        return false;

    return true;
}

VOID SVMUtilities::EnableSvmOnCore()
{
    __try
    {
        ULONGLONG vmcr = CPUUtilities::ReadMSR(0xC0010114);
        if (vmcr & (1ULL << 4))
        {
            DbgPrint("SVM locked on CPU %d\n", KeGetCurrentProcessorNumber());
            return;
        }

        ULONGLONG efer = CPUUtilities::ReadMSR(0xC0000080);
        if (!(efer & (1ULL << 12)))
        {
            efer |= (1ULL << 12);
            __writemsr(0xC0000080, efer);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("Exception enabling SVM on CPU %d: 0x%X\n",
            KeGetCurrentProcessorNumber(),
            GetExceptionCode());
    }
}
