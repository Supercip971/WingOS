#include "protocols/vfs/vfs.hpp"
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

FILE* fopen(const char* filename, const char* mode)
{
    (void)mode;
    prot::VfsConnection vfs = prot::VfsConnection::connect().unwrap();
    prot::FsFile file = vfs.open_path(core::Str(filename)).unwrap();
    FILE* f = new FILE();
    f->kind = FILE_KIND_FILE;
    f->file = core::move(file);
    
    f->cursor = 0;
    return f;

}
int remove(const char* filename)
{
    log::warn$("remove not implemented yet: {}", filename);
    // not implemented
    return 0;
}
int rename(const char* old_filename, const char* new_filename)
{
    log::warn$("rename not implemented yet: {} {}", old_filename, new_filename);
    return 0;
}

int puts(const char* str)
{
    size_t len = strlen(str);
    fwrite((void*)str, 1, len, stdout);
    char newline = '\n';
    fwrite((void*)&newline, 1, 1, stdout);
    return 0;
}

void fflush(FILE* stream)
{
    // no buffering, so nothing to do
    (void)stream;
}

int fclose(FILE* stream)
{
    switch(stream->kind)
    {
        case FILE_KIND_FILE: 
        {
            stream->file.close();
            delete stream;
            return 0;
        }
        case FILE_KIND_OUT: 
        {
            delete stream;
            return 0;
        }
        case FILE_KIND_IN:
        case FILE_KIND_VOID:
        default: 
        {
            return -1;  
        }
    }
}
int fseek(FILE* stream, long offset, int origin)
{
    switch(stream->kind)
    {
        case FILE_KIND_FILE: 
        {
            switch(origin)
            {
                case SEEK_SET:
                {
                    stream->cursor = offset;
                    return 0;
                }
                case SEEK_CUR:
                {
                    stream->cursor += offset;
                    return 0;
                }
                case SEEK_END:
                {
                    auto info = stream->file.get_info();
                    if (info.is_error())
                    {
                        return -1;
                    }
                    stream->cursor = info.unwrap().size + offset;
                    return 0;
                }
                default:
                {
                    return -1;  
                }
            }
        }
        case FILE_KIND_OUT: 
        
        case FILE_KIND_IN:
        case FILE_KIND_VOID:
        default: 
        {
            return -1;
        }
    }

}
long ftell(FILE* stream)
{
    switch(stream->kind)
    {
        case FILE_KIND_FILE: 
        {
            return stream->cursor;
        }
        case FILE_KIND_OUT: 
        
        case FILE_KIND_IN:
        case FILE_KIND_VOID:
        default: 
        {
            return -1;
        }
    }
}