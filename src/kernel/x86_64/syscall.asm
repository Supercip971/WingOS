%macro push_all 0

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

%endmacro

%macro pop_all_syscall 0

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx

%endmacro

extern syscall_higher_handler

global syscall_handle
syscall_handle:
    swapgs
    mov qword [gs:0x8], rsp       ; gs.saved_stack = rsp
    mov rsp, qword [gs:0x0]       ; rsp = gs.syscall_stack

    ; push information (gs, cs, rip, rflags, rip...)
    push qword 0x1b         ; user data segment
    push qword [gs:0x8]     ; saved stack
    push r11                ; saved rflags
    push qword 0x23         ; user code segment
    push rcx                ; current RIP

    cld
    push_all                ; push every register

    mov rdi, rsp            ; put the stackframe as the syscall argument
    mov rbp, 0

    call syscall_higher_handler ; jump to beautiful higher level code

    pop_all_syscall         ; pop every register except RAX as we use it for the return value



    mov rsp, qword [gs:0x8]



    swapgs
    o64 sysret
