#pragma once
#include <ntifs.h>

#define VMCB_SIZE 0x1000

#pragma pack(push, 1)

typedef struct _VMCB_CONTROL_AREA {
    UINT32 intercept_cr_read;
    UINT32 intercept_cr_write;
    UINT32 intercept_dr_read;
    UINT32 intercept_dr_write;
    UINT32 intercept_exceptions;
    UINT32 intercept_instr1;
    UINT32 intercept_instr2;
    UINT32 intercept_instr3;
    UINT8  reserved1[40];
    UINT16 pause_filter_threshold;
    UINT16 pause_filter_count;
    UINT64 iopm_base_pa;
    UINT64 msrpm_base_pa;
    UINT64 tsc_offset;
    UINT32 guest_asid;
    UINT32 tlb_control;
    UINT64 v_tpr;
    UINT64 v_irq;
    UINT64 v_intr_prio;
    UINT64 v_ign_tpr;
    UINT64 v_intr_masking;
    UINT64 v_intr_vector;
    UINT8  reserved2[24];
    UINT64 interrupt_shadow;
    UINT64 exit_code;
    UINT64 exit_info1;
    UINT64 exit_info2;
    UINT64 exit_int_info;
    UINT64 np_enable;
    UINT64 avic_apic_bar;
    UINT64 guest_pa_of_ghcb;
    UINT64 event_injection;
    UINT64 nested_cr3;
    UINT64 lbr_virtualization_enable;
    UINT64 vmcb_clean_bits;
    UINT64 next_rip;
    UINT8  num_of_bytes_fetched;
    UINT8  guest_instruction_bytes[15];
    UINT64 avic_apic_backing_page_pointer;
    UINT64 reserved3;
    UINT64 avic_logical_table_pointer;
    UINT64 avic_physical_table_pointer;
    UINT64 reserved4;
    UINT64 vmcb_save_state_pointer;
    UINT8  reserved5[752];
} VMCB_CONTROL_AREA;

typedef struct _VMCB_STATE_SAVE_AREA {
    UINT16 es_selector;
    UINT16 es_attrib;
    UINT32 es_limit;
    UINT64 es_base;

    UINT16 cs_selector;
    UINT16 cs_attrib;
    UINT32 cs_limit;
    UINT64 cs_base;

    UINT16 ss_selector;
    UINT16 ss_attrib;
    UINT32 ss_limit;
    UINT64 ss_base;

    UINT16 ds_selector;
    UINT16 ds_attrib;
    UINT32 ds_limit;
    UINT64 ds_base;

    UINT16 fs_selector;
    UINT16 fs_attrib;
    UINT32 fs_limit;
    UINT64 fs_base;

    UINT16 gs_selector;
    UINT16 gs_attrib;
    UINT32 gs_limit;
    UINT64 gs_base;

    UINT16 gdtr_selector;
    UINT16 gdtr_attrib;
    UINT32 gdtr_limit;
    UINT64 gdtr_base;

    UINT16 ldtr_selector;
    UINT16 ldtr_attrib;
    UINT32 ldtr_limit;
    UINT64 ldtr_base;

    UINT16 idtr_selector;
    UINT16 idtr_attrib;
    UINT32 idtr_limit;
    UINT64 idtr_base;

    UINT16 tr_selector;
    UINT16 tr_attrib;
    UINT32 tr_limit;
    UINT64 tr_base;

    UINT8  reserved1[43];
    UINT8  cpl;
    UINT32 reserved2;
    UINT64 efer;
    UINT8  reserved3[112];
    UINT64 cr4;
    UINT64 cr3;
    UINT64 cr0;
    UINT64 dr7;
    UINT64 dr6;
    UINT64 rflags;
    UINT64 rip;
    UINT8  reserved4[88];
    UINT64 rsp;
    UINT8  reserved5[24];
    UINT64 rax;
    UINT64 star;
    UINT64 lstar;
    UINT64 cstar;
    UINT64 sfmask;
    UINT64 kernel_gs_base;
    UINT64 sysenter_cs;
    UINT64 sysenter_esp;
    UINT64 sysenter_eip;
    UINT64 cr2;
    UINT8  reserved6[32];
    UINT64 g_pat;
    UINT64 dbgctl;
    UINT64 br_from;
    UINT64 br_to;
    UINT64 last_excp_from;
    UINT64 last_excp_to;
    UINT8  reserved7[80];
    UINT64 rcx;
    UINT64 rdx;
    UINT64 rbx;
} VMCB_STATE_SAVE_AREA;

typedef struct _VMCB {
    VMCB_CONTROL_AREA control_area;
    VMCB_STATE_SAVE_AREA state_save_area;
} VMCB;

#pragma pack(pop)

class VMCBUtilities
{
public:
    static VMCB* AllocateVmcb();
    static void FreeVmcb(VMCB* vmcb);
    static void SetupVMCB(VMCB* vmcb);
};