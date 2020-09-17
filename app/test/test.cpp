

int main(){

    asm volatile("mov rax, %0 \n"
                 "mov rdi, rax\n"
                 "mov rax, %1 \n"
                 "mov r10, rax \n"
                 "int 0x7f"

                 "": : "e"(0), "e"(32)); // for debugging
    while (true) {


    }
    return 0;
}
