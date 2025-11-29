#include "stdio.h"
#include <stdarg.h>

#include <string.h>

#include "stdio_fs.hpp"

extern "C"
size_t fwrite(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file )
{
    uint8_t* ptr_v = (uint8_t*)ptr;
    switch(file->kind)
    {
        case FILE_KIND_FILE: 
        {
            file->file.write(ptr, file->cursor, size * n);
            file->cursor += size * n;
            return n;
        }
        case FILE_KIND_OUT: 
        {
            for(size_t i = 0; i < size * n; i += 100)
            {

                if(size * n >= 100)
                {

                    file->output->send(ptr_v, 100);
                    ptr_v += 100;

                }
                else 
                {
                    file->output->send(ptr_v, size * n - i );
                    ptr_v += size * n - i ;
                }

                
            }

            return n;
        }
        case FILE_KIND_IN:
        case FILE_KIND_VOID:
        default: 
        {
            return -1;  
        }
    }
}

size_t fread(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file )

{
    uint8_t* ptr_v = (uint8_t*)ptr;
    switch(file->kind)
    {
        case FILE_KIND_FILE: 
        {
            file->file.read( ptr, file->cursor, size * n);
            file->cursor += size * n;
            return n;
        }
        case FILE_KIND_IN: 
        {
            for(size_t i = 0; i < size * n; i += 100)
            {

                if(size * n >= 100)
                {

                    file->input->receive(ptr_v, 100);
                    ptr_v += 100;

                }
                else 
                {
                    file->input->receive(ptr_v, size * n - i );
                    ptr_v += size * n - i ;
                }

                
            }

            return n;
        }
        case FILE_KIND_OUT:
        case FILE_KIND_VOID:
        default: 
        {
            return -1;  
        }
    }
}

