
bits 64
global gdtr_install
gdtr_install:
  lgdt [rdi]
  mov ax, 16
  mov ss, ax
  mov ds, ax
  mov es, ax
  mov rax, qword .trampoline
  push rsi
  push rax
  o64 retf
.trampoline:
  ret
