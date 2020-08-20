global tss_install
tss_install:
  push rbp
  mov rbp, rsp
  mov rax, rdi
  ltr ax
  pop rbp
  ret