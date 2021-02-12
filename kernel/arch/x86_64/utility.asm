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
    and rax, ~(1<<2)
    or rax, (1<<1)
    mov cr0, rax
    mov rax, cr4
    or rax, 3 << 9
    mov cr4, rax
    ret

global enable_xsave
enable_xsave:
    mov rax, cr4
    or rax, (1<<18)
    mov cr4, rax

    push rax
    push rcx
    push rdx
    xor rax,rax
    xor rcx, rcx
    mov rdx, 0
    or rax, 3
    xsetbv

    pop rdx
    pop rcx
    pop rax
    ret

global avx_init
avx_init:

    push rax
    push rcx
    push rdx
    xor rax,rax
    xor rcx, rcx
    mov rdx, 0
    or rax, 7
    xsetbv

    pop rdx
    pop rcx
    pop rax
    ret
global asm_sse_save
asm_sse_save:
    mov rax, rdi
    fxsave [rax]
    ret
global asm_sse_load
asm_sse_load:
    mov rax, rdi
    fxrstor [rax]
    ret

global asm_avx_save
asm_avx_save:
    mov eax, 0xffffffff
    mov edx, 0xffffffff
    xsave64 [rdi]
    ret
global asm_avx_load
asm_avx_load:
    mov eax, 0xffffffff
    mov edx, 0xffffffff
    xrstor64 [rdi]
    ret

read_pit_counter:
    mov al, 0x0
    out 0x43, al

    in al, 0x40
    mov ah, al
    in al, 0x40
    rol ax, 8
    ret
