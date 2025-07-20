int _start()
{
    while(true)
    {
        asm volatile("int $69");
        asm volatile("hlt");
    }
    return 1;
}