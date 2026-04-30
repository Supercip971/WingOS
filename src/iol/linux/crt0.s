section .text

extern _linux_start

global _cstart
_cstart:
    pop  rdi          ; rdi = argc
    mov  rsi, rsp     ; rsi = argv
    and  rsp, -16     ; rsp % 16 == 0
    call _linux_start
    ud2
.end:
