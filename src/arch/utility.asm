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
    cmp rax, 0x1ffff
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
