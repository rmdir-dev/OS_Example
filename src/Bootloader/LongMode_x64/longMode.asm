_CheckLongMode:
    pusha

    pushfd              ; push eflags
    pop eax             ; pop eflags into eax
    mov ecx, eax        ; save eax into ecx

    xor eax, 1 << 21    ; toggle the 21bit
                        ; 21st bit is "Able to use CPUID instruction (Pentium+)"
                        ; https://en.wikipedia.org/wiki/FLAGS_register

    push eax            ; put eax into the stack
    popfd               ; pop the eflags values
                        ; this will try to push the ID flag to 1 if it was 0

    pushfd              ; push the eflags values
    pop eax             ; put the eflags value into eax
                        ; here we recover the values, with the flipped ID

    push ecx            ; we restore the eflags back to their original state
    popfd               

    xor eax, ecx        ; compare eax with ecx
    jz .NoCPUID         ; if they are equal that mean that the CPUID bit
                        ; wasn't flipped, and thus CPUID is not supported

    mov eax, 0x80000000
    cpuid               ; This returns the CPU's manufacturer ID string 
                        ;   - a twelve-character ASCII string stored in EBX, EDX, ECX (in that order). 
                        ;   - The highest basic calling parameter (largest value that EAX can be set to before calling CPUID) is returned in EAX
    cmp eax, 0x80000001 ; check if eax is 
    jb .NoLongMode      ; check if eax is greater than 0x80000001 jb = jump below in unsigned mode
                        ; we can access extended CPU information.

    mov eax, 0x80000001 ; 
    cpuid               ; This returns the CPU's manufacturer ID string 
                        ;   - a twelve-character ASCII string stored in EBX, EDX, ECX (in that order). 
                        ;   - The highest basic calling parameter (largest value that EAX can be set to before calling CPUID) is returned in EAX
    test edx, 1 << 29   ; test if the 29th bit of edx is set. 
                        ; the 29th bit is the LM-Bit 
    jz .NoLongMode      ; if the 29th bit is not set then longmode is not supported.
                        ; long mode supported

    PrintStringNextLine LM_SUPPORTED
    popa
    ret

.NoLongMode:
    PrintStringNextLine NO_LM_MSG
    jmp .ErrorLoop
.NoCPUID:
    PrintStringNextLine NO_CUPID_MSG

.ErrorLoop:
    jmp $

NO_CUPID_MSG:
    db "Err CUPID!",0

NO_LM_MSG:
    db "Err x64",0

LM_SUPPORTED:
    db "Long mode supported!", 0