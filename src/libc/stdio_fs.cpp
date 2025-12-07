#include "protocols/vfs/vfs.hpp"
#include "stdio.h"
#include <stdarg.h>

#include <string.h>

#include "stdio_fs.hpp"

extern "C"
size_t fwrite(void* __restrict ptr, size_t size, size_t n, FILE* __restrict file )
{
    if(file == nullptr)
    {
        log::log$("fwrite: file is null");
        return 0;
    }
    uint8_t* ptr_v = (uint8_t*)ptr;
    switch(file->kind)
    {
        case FILE_KIND_FILE: 
        {
            file->file->write(ptr, file->cursor, size * n);
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
        case FILE_KIND_WRITER:
        {
            size_t total_written = 0;
            while (total_written < size * n)
            {
                auto res = file->writer->write((const char*)ptr_v + total_written, size * n - total_written);
                if (res.is_error())
                {
                    return total_written / size;
                }
                total_written += size * n - total_written;
            }
            return n;
        }
        case FILE_KIND_READER:
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
            file->file->read( ptr, file->cursor, size * n);
            file->cursor += size * n;
            return n;
        }
        case FILE_KIND_READER:
        {
            size_t total_read = 0;
            while (total_read < size * n)
            {
                auto res = file->reader->read((char*)ptr_v + total_read, size * n - total_read);
                if (res.is_error())
                {
                    return total_read / size;
                }
                total_read += size * n - total_read;
            }
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
        case FILE_KIND_WRITER:
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
    prot::FsFile* file = new prot::FsFile(core::move(vfs.open_path(core::Str(filename)).unwrap()));
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
int mkdir(const char* pathname, unsigned int mode)
{
    log::warn$("mkdir not implemented yet: {} {}", pathname, mode);
    return 0;
}
int fclose(FILE* stream)
{
    switch(stream->kind)
    {
        case FILE_KIND_FILE: 
        {
            stream->file->close();
            delete stream->file;
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
        case FILE_KIND_READER:
        case FILE_KIND_WRITER:
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
                    auto info = stream->file->get_info();
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
        case FILE_KIND_READER:
        case FILE_KIND_WRITER:
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
        case FILE_KIND_READER:
        case FILE_KIND_WRITER:
        default: 
        {
            return -1;
        }
    }
}

int feof(FILE* stream)
{
    if (stream == nullptr)
        return 1;
    return stream->eof_flag;
}

int ferror(FILE* stream)
{
    if (stream == nullptr)
        return 1;
    return stream->error_flag;
}

int fgetc(FILE* stream)
{
    if (stream == nullptr)
        return -1;
    
    // Check for ungotten character first
    if (stream->ungetc_buf != -1)
    {
        int c = stream->ungetc_buf;
        stream->ungetc_buf = -1;
        return c;
    }
    
    unsigned char c;
    size_t read = fread(&c, 1, 1, stream);
    if (read != 1)
    {
        stream->eof_flag = 1;
        return -1;  // EOF
    }
    return c;
}

int ungetc(int c, FILE* stream)
{
    if (stream == nullptr || c == -1)
        return -1;
    
    // Can only unget one character
    if (stream->ungetc_buf != -1)
        return -1;
    
    stream->ungetc_buf = c;
    stream->eof_flag = 0;  // Clear EOF flag
    return c;
}
