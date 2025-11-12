#include <ntifs.h>
#include <intrin.h>

#define MSR_VM_CR 0xC0010114
#define VM_CR_SVMDIS (1ULL << 4)

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Driver unloaded.\n");
}

VOID GetCpuid(int leaf, int* eax, int* ebx, int* ecx, int* edx)
{
    int cpuInfo[4];
    __cpuid(cpuInfo, leaf);
    *eax = cpuInfo[0];
    *ebx = cpuInfo[1];
    *ecx = cpuInfo[2];
    *edx = cpuInfo[3];
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    DriverObject->DriverUnload = DriverUnload;

    int eax, ebx, ecx, edx;
    char vendor[13];

    GetCpuid(0, &eax, &ebx, &ecx, &edx);
    *(int*)&vendor[0] = ebx;
    *(int*)&vendor[4] = edx;
    *(int*)&vendor[8] = ecx;
    vendor[12] = '\0';

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "CPU Vendor: %s\n", vendor);

    GetCpuid(1, &eax, &ebx, &ecx, &edx);

    BOOLEAN svm = FALSE;

    if (strcmp(vendor, "AuthenticAMD") == 0)
    {
        GetCpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        svm = (ecx & (1 << 2)) != 0;
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "SVM Support: %s\n", svm ? "Yes" : "No");
    }
    else
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Unknown CPU vendor.\n");
    }

    ULONG64 vmcr = __readmsr(MSR_VM_CR);

    if (vmcr & VM_CR_SVMDIS)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SVM is DISABLED by BIOS.\n");
    else
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "SVM appears ENABLED in BIOS.\n");

    return STATUS_SUCCESS;
}
