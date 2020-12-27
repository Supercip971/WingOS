#include <64bit.h>
#include <interrupt.h>
void dump_register(InterruptStackFrame *stck)
{
    // this is the least readable code EVER
    printf(" ===== cpu dump ===== \n");
    printf(" ===== cs and ss ===== \n");

    printf("cs = %x | ss = %x \n", stck->cs, stck->ss);
    printf(" ===== utility ===== \n");
    printf("rsp = %x | rbp = %x | rdi = %x \n", stck->rsp, stck->rbp, stck->rdi);
    printf("rsi = %x | rdx = %x | rcx = %x \n", stck->rsi, stck->rdx, stck->rcx);
    printf("rbx = %x | rax = %x |  \n", stck->rbx, stck->rax);
    printf(" ===== other ===== \n");
    printf("error code = %x \n", stck->error_code);
    printf("interrupt number = %x \n", stck->int_no);
    printf("rip = %x \n", stck->rip);
    printf("flags = %x \n", stck->rflags);

    uintptr_t CRX;
    asm volatile("mov %0, cr2"
                 : "=r"(CRX));
    printf("CR2 = %x \n", CRX);
    asm volatile("mov %0, cr3"
                 : "=r"(CRX));
    printf("CR3 = %x \n", CRX);
    asm volatile("mov %0, cr4"
                 : "=r"(CRX));
    printf("CR4 = %x \n", CRX);
}
