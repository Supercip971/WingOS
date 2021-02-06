global reload_cr3
reload_cr3:
    mov rax, cr3
    mov cr3, rax
    ret
global read_pit_counter


global sse_init
; sse 1 and 2 should be supported on any 64bit processor
sse_init:
    mov rax, cr0
    and ax, 0xfffb
    or ax, 0x2
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9
    mov cr4, rax
    ret


global asm_sse_save
asm_sse_save:
    mov rax, rdi
    FXRSTOR64 [rax]
    ret
global asm_sse_load
asm_sse_load:

    mov rax, rdi
    FXRSTOR64 [rax]
    ret


read_pit_counter:
    mov al, 0x0
    out 0x43, al

    in al, 0x40
    mov ah, al
    in al, 0x40
    rol ax, 8
    ret
