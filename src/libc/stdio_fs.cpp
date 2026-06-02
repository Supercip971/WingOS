#include <stdarg.h>
#include <string.h>

#include "stdio_fs.hpp"

#include "protocols/vfs/vfs.hpp"
#include "stdio.h"

extern "C" size_t fwrite(const void *__restrict ptr, size_t size, size_t n, FILE *__restrict file)
{
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
    if (!file)
    {
        fmt::log$("fwrite: file is null");
        return 0;
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    if (file->kind == FILE_KIND_VOID)
    {
        fmt::err$("fwrite: file handle is invalid (use-after-fclose detected!)");
        return 0;
    }
    uint8_t *ptr_v = (uint8_t *)ptr;
    switch (file->kind)
    {
    case FILE_KIND_FILE:
    {
        file->file->write(const_cast<void *>(ptr), file->cursor, size * n);
        file->cursor += size * n;
        return n;
    }
    case FILE_KIND_OUT:
    {
        for (size_t i = 0; i < size * n; i += 100)
        {

            if (size * n >= 100)
            {

                file->output->send(ptr_v, 100);
                ptr_v += 100;
            }
            else
            {
                file->output->send(ptr_v, size * n - i);
                ptr_v += size * n - i;
            }
        }

        return n;
    }
    case FILE_KIND_WRITER:
    {
        size_t total_written = 0;
        while (total_written < size * n)
        {
            auto res = file->writer->write((const char *)ptr_v + total_written, size * n - total_written);
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

size_t fread(void *__restrict ptr, size_t size, size_t n, FILE *__restrict file)

{
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
    if (!file)
    {
        fmt::err$("fread: file is null");
        return 0;
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

    uint8_t *ptr_v = (uint8_t *)ptr;
    switch (file->kind)
    {
    case FILE_KIND_FILE:
    {
        size_t total = size * n;
        uint8_t *dst = (uint8_t *)ptr;
        size_t done = 0;

        // Drain ungetc buffer
        if (file->ungetc_buf != -1 && total > 0)
        {
            dst[0] = (uint8_t)file->ungetc_buf;
            file->ungetc_buf = -1;
            done = 1;
        }

        if (done < total)
        {
            if (file->cursor >= file->end)
            {
                file->eof_flag = 1;
                return done / size;
            }
            // Clamp to the data actually present in the file so FsFile::read
            // never requests bytes beyond EOF (the IPC returned byte-count is
            // unreliable, so FsFile::read always copies the full `len` it receives).
            size_t available = file->end - file->cursor;
            size_t to_read = (total - done < available) ? (total - done) : available;

            fmt::log$("from size: {} to reading {} + {}", size*n, to_read, done);
            auto read_result = file->file->read(dst + done, file->cursor, to_read);
            if (!read_result.is_error())
            {
                size_t bytes_read = read_result.unwrap();
                file->cursor += bytes_read;
                done += bytes_read;
            }
            file->eof_flag = (file->cursor >= file->end) ? 1 : 0;
        }
        return done / size;
    }
    case FILE_KIND_READER:
    {
        size_t total_read = 0;
        while (total_read < size * n)
        {
            auto res = file->reader->read((char *)ptr_v + total_read, size * n - total_read);
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
        for (size_t i = 0; i < size * n; i += 100)
        {

            if (size * n >= 100)
            {

                file->input->receive(ptr_v, 100);
                ptr_v += 100;
            }
            else
            {
                file->input->receive(ptr_v, size * n - i);
                ptr_v += size * n - i;
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

FILE *fopen(const char *filename, const char *mode)
{
    (void)mode;

    auto vfs_res = prot::VfsConnection::connect();
    if (vfs_res.is_error())
    {
        fmt::err$("fopen: failed to connect to VFS: {}", vfs_res.error());
        return nullptr;
    }
    prot::VfsConnection vfs = core::move(vfs_res.unwrap());

    auto path_res = vfs.open_path(core::Str(filename));
    if (path_res.is_error())
    {
        fmt::err$("fopen: failed to open path {}: {}", filename, path_res.error());
        return nullptr;
    }

    prot::FsFile *file = new prot::FsFile(core::move(path_res.unwrap()));

    FILE *f = new FILE();
    f->kind = FILE_KIND_FILE;
    f->buffer = core::WStr::copy(filename);
    f->file = file;

    f->end = file->get_info().unwrap().size;
    f->eof_flag = 0;
    f->error_flag = 0;
    f->ungetc_buf = -1;
    f->cursor = 0;
    return f;
}
int remove(const char *filename)
{
    fmt::warn$("remove not implemented yet: {}", filename);
    // not implemented
    return 0;
}
int rename(const char *old_filename, const char *new_filename)
{
    fmt::warn$("rename not implemented yet: {} {}", old_filename, new_filename);
    return 0;
}

int puts(const char *str)
{
    size_t len = strlen(str);
    fwrite((void *)str, 1, len, stdout);
    char newline = '\n';
    fwrite((void *)&newline, 1, 1, stdout);
    return 0;
}

void fflush(FILE *stream)
{
    // no buffering, so nothing to do
    (void)stream;
}
int mkdir(const char *pathname, unsigned int mode)
{
    fmt::warn$("mkdir not implemented yet: {} {}", pathname, mode);
    return 0;
}

int fclose(FILE *stream)
{
    if (stream == nullptr)
    {
        fmt::err$("fclose: stream is null");
        return -1;
    }
    if (stream->kind == FILE_KIND_VOID)
    {
        fmt::err$("fclose: file already closed (double-fclose detected!)");
        return -1;
    }
    fmt::log$("calling fclose: {}", stream->buffer.view());
    switch (stream->kind)
    {
    case FILE_KIND_FILE:
    {
        stream->file->close();

        delete stream->file;
        stream->file = nullptr;
        // Mark as void instead of deleting to detect use-after-free
        stream->kind = FILE_KIND_VOID;
        // Note: intentionally not deleting stream to catch use-after-free bugs
        // In production, you may want to delete it after debugging
        return 0;
    }
    case FILE_KIND_OUT:
    {
        stream->kind = FILE_KIND_VOID;
        return 0;
    }
    case FILE_KIND_IN:
    case FILE_KIND_READER:
    case FILE_KIND_WRITER:
    {
        stream->kind = FILE_KIND_VOID;
        return 0;
    }
    case FILE_KIND_VOID:
    default:
    {
        return -1;
    }
    }
}
int fseek(FILE *stream, long offset, int origin)
{
    if (stream == nullptr)
    {
        fmt::err$("fseek: stream is null");
        return -1;
    }
    if (stream->kind == FILE_KIND_VOID)
    {
        fmt::err$("fseek: file handle is invalid (use-after-fclose detected!)");
        return -1;
    }

    stream->ungetc_buf = -1;
    switch (stream->kind)
    {
    case FILE_KIND_FILE:
    {
        switch (origin)
        {
        case SEEK_SET:
        {
            stream->cursor = offset;
            if(stream->cursor >= stream->end)
            {
                stream->eof_flag = 1;
            }
            else
            {
                stream->eof_flag = 0;
            }
            return 0;
        }
        case SEEK_CUR:
        {
            stream->cursor += offset;
            stream->eof_flag = (stream->cursor >= stream->end) ? 1 : 0;
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
            stream->eof_flag = 1;

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
long ftell(FILE *stream)
{
    if (stream == nullptr)
    {
        fmt::err$("ftell: stream is null");
        return -1;
    }
    if (stream->kind == FILE_KIND_VOID)
    {
        fmt::err$("ftell: file handle is invalid (use-after-fclose detected!)");
        return -1;
    }
    switch (stream->kind)
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

int feof(FILE *stream)
{
    if (stream == nullptr)
        return 1;
    return stream->eof_flag;
}

int ferror(FILE *stream)
{
    if (stream == nullptr)
        return 1;
    return stream->error_flag;
}

int fgetc(FILE *stream)
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
        return -1; // EOF
    }
    stream->eof_flag = 0;
    return c;
}

int ungetc(int c, FILE *stream)
{
    if (stream == nullptr || c == -1)
        return -1;

    // Can only unget one character
    if (stream->ungetc_buf != -1)
        return -1;

    stream->ungetc_buf = c;
    stream->eof_flag = 0; // Clear EOF flag
    return c;
}
