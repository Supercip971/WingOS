const char* hello_world = "Hello, World!\n";

extern"C" int _start()
{
    while(true)
    {
        unsigned long kernel_return;
        asm volatile(
        "push %%r11 \n"
        "push %%rcx \n"
            "syscall \n"
        "pop %%rcx \n"
        "pop %%r11 \n"
            : "=a"(kernel_return)
            : "a"(0), "b"(hello_world), "d"(0), "S"(0), "D"(0)
            : "memory", "rcx", "r11"); // for debugging
   
    }
    return 1;
}