global reload_cr3
reload_cr3:
    mov rax, cr3
    mov cr3, rax
    ret
global asm_spinlock_lock
global asm_spinlock_unlock
extern something_is_bad_i_want_to_die_higher_level ;
asm_spinlock_lock:
    mov rax, 0
    lock bts dword [rdi], 0
    jc spin ; wait
    ret

spin: ; never gonna lock you up never gonna lock you doooown
    inc rax
    cmp rax, 0xffffff
    je something_is_bad_i_want_to_die
    pause
    test dword [rdi], 1 ; finnaly the lock is not here
    jnz spin ; redo again
    jmp asm_spinlock_lock ; so redo it and lock it



something_is_bad_i_want_to_die:

    call something_is_bad_i_want_to_die_higher_level
    ret
asm_spinlock_unlock:
    lock btr dword [rdi], 0 ; Set the bit to 0
    ret  ; unlocking :D



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
    fxsave [eax]
    ret
global asm_sse_load
asm_sse_load:

    mov rax, rdi
    fxrstor [eax]
    ret
