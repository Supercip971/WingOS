
global idt_use
idt_use: 
    lidt [rdi] 
    ret