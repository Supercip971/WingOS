section .text

extern _linux_start

global _cstart
_cstart:
    push rax
    call _linux_start
    ud2
.end: