
bits 64
global gdt_flush
gdt_flush: 
    mov ax, 0x10 ; We can't write to the segment registers directly
    mov ds, ax
    mov es, ax
    mov ax, 0x0
    mov ss, ax
    mov ax, 0x10
    mov gs, ax
    mov fs, ax
    ret
   