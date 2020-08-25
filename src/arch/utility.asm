global reload_cr3
reload_cr3:
    mov rax, cr3
    mov cr3, rax
    ret
