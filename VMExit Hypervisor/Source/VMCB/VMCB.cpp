#include <Includes.h>

extern "C" UINT16 AsmReadCS();
extern "C" UINT16 AsmReadDS();
extern "C" UINT16 AsmReadES();
extern "C" UINT16 AsmReadSS();
extern "C" UINT16 AsmReadFS();
extern "C" UINT16 AsmReadGS();
extern "C" UINT16 AsmReadTR();
extern "C" UINT16 AsmReadLDTR();
extern "C" UINT64 AsmGetSegmentAttrib(UINT16 selector);
extern "C" void AsmReadGDT(void* gdtr);
extern "C" void AsmReadIDT(void* idtr);

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

void VMCBUtilities::SetupVMCB(VMCB* vmcb)
{
    if (!vmcb) return;

    RtlZeroMemory(vmcb, VMCB_SIZE);

    // -------------------------
    // CONTROL AREA: intercepts
    // -------------------------
    vmcb->control_area.intercept_instr1 =
        (1 << 18) | // CPUID
        (1 << 24);  // HLT

    vmcb->control_area.intercept_cr_read = 0;
    vmcb->control_area.intercept_cr_write = 0;
    vmcb->control_area.intercept_dr_read = 0;
    vmcb->control_area.intercept_dr_write = 0;
    vmcb->control_area.intercept_exceptions = 0;
    vmcb->control_area.guest_asid = 1;
    vmcb->control_area.np_enable = 0;

    // -------------------------
    // SEGMENTS
    // -------------------------
    UINT16 cs = AsmReadCS();
    UINT16 ds = AsmReadDS();
    UINT16 es = AsmReadES();
    UINT16 ss = AsmReadSS();
    UINT16 fs = AsmReadFS();
    UINT16 gs = AsmReadGS();
    UINT16 tr = AsmReadTR();
    UINT16 ldtr = AsmReadLDTR();

    // -------------------------
    // DESCRIPTOR TABLES
    // -------------------------
    struct DESCRIPTOR_TABLE_REG { UINT16 Limit; UINT64 Base; };
    DESCRIPTOR_TABLE_REG gdtr = { 0 }, idtr = { 0 };
    AsmReadGDT(&gdtr);
    AsmReadIDT(&idtr);

    auto GetAttrib = [](UINT16 selector) -> UINT16 {
        if (selector == 0) return 0;
        return (UINT16)AsmGetSegmentAttrib(selector);
        };

    // CS
    vmcb->state_save_area.cs_selector = cs;
    vmcb->state_save_area.cs_attrib = GetAttrib(cs);
    vmcb->state_save_area.cs_limit = 0xFFFFFFFF;
    vmcb->state_save_area.cs_base = 0;

    // DS
    vmcb->state_save_area.ds_selector = ds;
    vmcb->state_save_area.ds_attrib = GetAttrib(ds);
    vmcb->state_save_area.ds_limit = 0xFFFFFFFF;
    vmcb->state_save_area.ds_base = 0;

    // ES
    vmcb->state_save_area.es_selector = es;
    vmcb->state_save_area.es_attrib = GetAttrib(es);
    vmcb->state_save_area.es_limit = 0xFFFFFFFF;
    vmcb->state_save_area.es_base = 0;

    // SS
    vmcb->state_save_area.ss_selector = ss;
    vmcb->state_save_area.ss_attrib = GetAttrib(ss);
    vmcb->state_save_area.ss_limit = 0xFFFFFFFF;
    vmcb->state_save_area.ss_base = 0;

    // FS
    vmcb->state_save_area.fs_selector = fs;
    vmcb->state_save_area.fs_attrib = GetAttrib(fs);
    vmcb->state_save_area.fs_limit = 0xFFFFFFFF;
    vmcb->state_save_area.fs_base = __readmsr(0xC0000100);

    // GS
    vmcb->state_save_area.gs_selector = gs;
    vmcb->state_save_area.gs_attrib = GetAttrib(gs);
    vmcb->state_save_area.gs_limit = 0xFFFFFFFF;
    vmcb->state_save_area.gs_base = __readmsr(0xC0000101);

    // LDTR
    vmcb->state_save_area.ldtr_selector = ldtr;
    vmcb->state_save_area.ldtr_attrib = GetAttrib(ldtr);
    vmcb->state_save_area.ldtr_limit = 0;
    vmcb->state_save_area.ldtr_base = 0;

    // TR
    vmcb->state_save_area.tr_selector = tr;
    vmcb->state_save_area.tr_attrib = GetAttrib(tr);
    vmcb->state_save_area.tr_limit = 0x67;
    vmcb->state_save_area.tr_base = 0;

    // GDTR / IDTR
    vmcb->state_save_area.gdtr_limit = gdtr.Limit;
    vmcb->state_save_area.gdtr_base = gdtr.Base;
    vmcb->state_save_area.idtr_limit = idtr.Limit;
    vmcb->state_save_area.idtr_base = idtr.Base;

    // -------------------------
    // CONTROL REGISTERS
    // -------------------------
    vmcb->state_save_area.cr0 = __readcr0();
    vmcb->state_save_area.cr3 = __readcr3();
    vmcb->state_save_area.cr4 = __readcr4();
    vmcb->state_save_area.cr2 = 0; // don't read CR2

    // DEBUG REGISTERS
    vmcb->state_save_area.dr6 = __readdr(6);
    vmcb->state_save_area.dr7 = __readdr(7);

    // EFER
    vmcb->state_save_area.efer = __readmsr(0xC0000080);

    // RFLAGS
    vmcb->state_save_area.rflags = __readeflags();

    // CPL
    vmcb->state_save_area.cpl = cs & 3;

    // PAT
    vmcb->state_save_area.g_pat = __readmsr(0x277);

    // SYSCALL MSRs
    vmcb->state_save_area.star = __readmsr(0xC0000081);
    vmcb->state_save_area.lstar = __readmsr(0xC0000082);
    vmcb->state_save_area.cstar = __readmsr(0xC0000083);
    vmcb->state_save_area.sfmask = __readmsr(0xC0000084);
    vmcb->state_save_area.kernel_gs_base = __readmsr(0xC0000102);
}