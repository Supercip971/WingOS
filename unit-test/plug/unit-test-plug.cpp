
static int page_size = -1;
    #include <system_plug.h>
    #include <stdlib.h>
    #include <stdio.h>

    #include <string.h>
    #include <pthread.h>

    #include <unistd.h>

    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <stdlib.h>
    #include <unistd.h>
extern bool try_to_exit;


    void plug_init(){
        stdin = fopen("/dev/stdin", "r+");
        stdout = fopen("/dev/stdout", "r+");
        stderr = fopen("/dev/stderr", "r+");
    };
    void plug_end(){
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
    };
    uintptr_t plug_allocate_page(size_t count)
    {
        if ( page_size < 0 ) page_size = 4096;
        size_t size = count * page_size;

        char *p2 = (char*)mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0);
        if ( p2 == MAP_FAILED) return 0;

        if(mprotect(p2, size, PROT_READ|PROT_WRITE) != 0)
        {
            munmap(p2, size);
            return 0;
        }

        return (uintptr_t)p2;
    }
    bool plug_free_page(uintptr_t addr, size_t count)
    {
         munmap( (void*)addr, count * page_size );
        return true;
    }
    void plug_debug_out(const char *str, size_t length)
    {
        char* str2 = (char*)malloc(length+2);
        memcpy(str2,str, length+1);
        str2[length+1] = 0;
        printf(str2);
        free(str2);
    }

    int plug_open(const char *path_name, int flags, int mode)
    {
        printf("open plug \n");
        return syscall(SYS_open, path_name, flags, mode);
    }
    int plug_close(int fd)
    {
        
        printf("close plug \n");
        return syscall(SYS_close, fd);
    }
    size_t plug_lseek(int fd, size_t offset, int whence)
    {
        printf("seek plug \n");
        return syscall(SYS_lseek, fd, offset, whence);
    }
    size_t plug_read(int fd, void *buffer, size_t count)
    {
        printf("read plug \n");
        return syscall(SYS_read, fd, buffer, count);
    }
    void plug_exit(int)
    {
        try_to_exit = true;
        printf("exit plug \n");
    }
