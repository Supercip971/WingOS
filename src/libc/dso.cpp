extern"C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle)
{
    (void)destructor;
    (void)arg;
    (void)__dso_handle;
    return 0;
}
void *__dso_handle;

extern "C" void __cxa_finalize(void *f)
{
    (void)f;
}

