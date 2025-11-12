.code

; Read segment selectors
AsmReadCS PROC
    mov ax, cs
    ret
AsmReadCS ENDP

AsmReadDS PROC
    mov ax, ds
    ret
AsmReadDS ENDP

AsmReadES PROC
    mov ax, es
    ret
AsmReadES ENDP

AsmReadSS PROC
    mov ax, ss
    ret
AsmReadSS ENDP

AsmReadFS PROC
    mov ax, fs
    ret
AsmReadFS ENDP

AsmReadGS PROC
    mov ax, gs
    ret
AsmReadGS ENDP

AsmReadTR PROC
    str ax
    ret
AsmReadTR ENDP

AsmReadLDTR PROC
    sldt ax
    ret
AsmReadLDTR ENDP

; Read segment attributes (LAR instruction)
AsmGetSegmentAttrib PROC
    lar rax, rcx
    jnz no_attrib
    shr rax, 8
    ret
no_attrib:
    xor rax, rax
    ret
AsmGetSegmentAttrib ENDP

; Read descriptor tables
AsmReadGDT PROC
    sgdt [rcx]
    ret
AsmReadGDT ENDP

AsmReadIDT PROC
    sidt [rcx]
    ret
AsmReadIDT ENDP

; VMRUN wrapper - executes VMRUN and returns on VMEXIT
; RCX = Physical address of VMCB
AsmVmrun PROC
    ; Save host general purpose registers
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; RCX contains VMCB physical address
    mov rax, rcx
    
    ; Load guest state from VMCB
    vmload rax
    
    ; Execute VMRUN - enters guest mode
    vmrun rax
    
    ; We return here on VMEXIT
    ; Save guest state back to VMCB
    vmsave rax
    
    ; Restore host registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    
    ret
AsmVmrun ENDP

GuestEntryPoint PROC
    cli             ; disable interrupts
spin:
    hlt             ; CPU halts until next interrupt
    jmp spin
GuestEntryPoint ENDP

END