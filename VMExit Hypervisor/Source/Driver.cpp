#include <Includes.h>

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    Hypervisor::StopVmmThreads();

    DbgPrint("Hypervisor Unloaded\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("Hypervisor Loading\n");

    if (!SVMUtilities::IsSvmEnabled())
        return STATUS_UNSUCCESSFUL;

    DbgPrint("SVM Is Enabled\n");

    if (!NT_SUCCESS(Hypervisor::StartVmmThreads()))
        return STATUS_SUCCESS;

    DbgPrint("Started VMM Threads\n");

    return STATUS_SUCCESS;
}