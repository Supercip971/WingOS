
[bits 16]


TRAMPOLINE_BASE equ 0x1000

MSR_EFER equ 0xC0000080

EFER_LM equ 0x100
EFER_NX equ 0x800

CR0_PE equ 0x1
CR0_PAGING equ 0x80000000

CR4_PAE equ 0x20
CR4_PSE equ 0x10
; --- 16 BIT ---
global nstack
extern cpuupstart
global trampoline_start
trampoline_start:

    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

mov al, 'a'
mov dx, 0x3F8
out dx, al

    o32 lidt [pm_idtr - trampoline_start + TRAMPOLINE_BASE]
    o32 lgdt [pm_gdtr - trampoline_start + TRAMPOLINE_BASE]


mov al, 'a'
mov dx, 0x3F8
out dx, al
    mov eax, cr0
    or al, CR0_PE
    mov cr0, eax


mov al, 'c'
mov dx, 0x3F8
out dx, al
    jmp 0x8:(trampoline32 - trampoline_start + TRAMPOLINE_BASE)


[bits 32]


section .text
; ---- 32 BITS ----
trampoline32:

    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov ss, bx


    mov eax, dword 0x14000
    ; pire fix de ma vie, mais ça fonctionne, pourquoi ? je ne sais pas. est ce que je demande si ta grand mère fait du vélo ? NON alors MOILA >:( 
    ; pourquoi je devrai débatre sur un fix ? JAI PASSE PLUSIEURS JOUR POUR CE TRUC ALORS TU ME TRIGGER PAS
    ; et je code comme je veux OK ? est ce que je suis salé ? oui ! POURQUOI j'ai pas fait ça plus tôt ?
    ; bon au moins ce morceaux de scotch tient et moila
    ; toute personne du discord devse critiquant ce bout de code serra ban automatiquement
    ; après je peut faire plusieurs ligne
    ; c'est vrai que ce code est mauvait, charger la table de paging comme ça, ça devrait pas être légal
    ; mais une addresse fixe c'est une addresse fixe
    ; dailleurs aime tu les cookies ?
    ; non mais sérieux je crois que ça fait plusieurs jour que je bosse dessus j'ai juste envie de mourrir
    ; est-ce que github autorise ce code ?
    ; meh ça m'étonnerai, sinon je ban github du discord devse
    ; hmmm j'imagine bien un anglais traduire cette phrase en ne comprennant pas le code et en me
    ; voyant rager comme un gamins de 3 ans et demi
    ; moi quand j'ai commencé le développement on me disait
    ; << c'est fun >>
    ; BAH PARFOIS NON ! (oui révélation de ouf guedin)
    ; et j'ai juste envie d'apprendre et de coder pas de réfléchir pendant plusieurs jour juste pour un truc a la c
    ; surtout que l'osdev est l'un des truc les plus durs en programmation, quand tu reviens sur de la programmation normale les librairies standarts sont tellement bien
    ; après bon ça fait déjà 14 lignes de commentaires inutiles
    ; donc je vais m'expliquer pour ce fix
    ;
    ; c'était l'explication
    ; merci pour la lecture de ce paragraphe
    ; (que l'on viennent pas me critiquer pour le manque de commentaire dans le code, car sinon je vais te commenter moi hein !)
    ; bon faut vraiment que je réfléchisse à quelque chose d'intelligent à faire ...
    ; AH et aussi le système de logging pour le smp est merdique je sais michel

    mov cr3, eax

mov al, 'z'
mov dx, 0x3F8
out dx, al

    mov eax, cr4
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).

or eax, 1 << 7
mov cr4, eax



    mov ecx, 0xc0000080
    rdmsr

    or eax,1 << 8  ; LME
    wrmsr

    ; why not ?



mov al, 't'
mov dx, 0x3F8
out dx, al


    mov eax, cr0

    or eax, 1 << 31

    mov cr0, eax


mov al, '2'
mov dx, 0x3F8
out dx, al


    lgdt [lm_gdtr - trampoline_start + TRAMPOLINE_BASE]
    jmp 8:(trampoline64 - trampoline_start + TRAMPOLINE_BASE)



[bits 64]
; ---- 64 BITS ----
trampoline64:

mov al, '5'
mov dx, 0x3F8
out dx, al
mov al, 'd'
mov dx, 0x3F8
out dx, al
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ax, 0x0
    mov fs, ax
    mov gs, ax


mov al, 'p'
mov dx, 0x3F8
out dx, al


lgdt [0x580]
lidt [0x590]

mov rsp, 0x8FF0
    mov al, '6'
    mov dx, 0x3F8
    out dx, al

mov rbp, 0x0 ; terminate stack traces here
; reset RFLAGS
push 0x0
popf

mov al, '7'
mov dx, 0x3F8
out dx, al
mov rax, qword vcode64
    jmp vcode64

vcode64:
mov al, '8'
mov dx, 0x3F8
out dx, al

    push rbp
mov rax, qword trampoline_ext
call rax

; ---- LONG MODE ----
align 16
  lm_gdtr:
    dw lm_gdt_end - lm_gdt_start - 1
    dq lm_gdt_start - trampoline_start + TRAMPOLINE_BASE

align 16
  lm_gdt_start:
    ; null selector
    dq 0
    ; 64bit cs selector
    dq 0x00AF98000000FFFF
    ; 64bit ds selector
    dq 0x00CF92000000FFFF
  lm_gdt_end:


; ---- Protected MODE ----
align 16
  pm_gdtr:
    dw pm_gdt_end - pm_gdt_start - 1
    dd pm_gdt_start - trampoline_start + TRAMPOLINE_BASE

align 16
  pm_gdt_start:
    ; null selector
    dq 0
    ; cs selector
    dq 0x00CF9A000000FFFF
    ; ds selector
    dq 0x00CF92000000FFFF
  pm_gdt_end:
; IDT
align 16
  pm_idtr:
    dw 0
    dd 0
    dd 0
    align 16

nstack: ; variable for storing the virtual address of the AP's bootstrap stack
    dq 0

global trampoline_end
trampoline_end:

trampoline_ext:

mov al, '8'
mov dx, 0x3F8
out dx, al
    call cpuupstart


