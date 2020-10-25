#include <klib/kernel_file_system.h>
#include <klib/process_message.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
namespace sys
{

    uint64_t file_open(const char *path, const char *right)
    {

        sys::file_system_service_protocol dat = {0};
        dat.request_type = FILE_OPEN;
        memcpy(dat.open.file_directory, path, strlen(path) + 1);
        memcpy(dat.open.mode, right, strlen(right) + 1);
        dat.open.file_request_id = 0; // for later re opening
        uint64_t result = sys::process_message("file_system_service", (uint64_t)&dat, sizeof(sys::file_system_service_protocol)).read();
        return result;
    }
    void file_close(uint64_t fid)
    {
        sys::file_system_service_protocol dat = {0};
        dat.request_type = FILE_CLOSE;
        dat.close.file_request_id = fid;

        sys::process_message("file_system_service", (uint64_t)&dat, sizeof(sys::file_system_service_protocol)).read();
    }
    file_information get_file_information(uint64_t fid)
    {
        sys::file_system_service_protocol dat = {0};
        file_information *fi = reinterpret_cast<file_information *>(malloc(sizeof(file_information)));
        dat.request_type = GET_FILE_INFO;
        dat.info.file_request_id = fid;
        dat.info.target = fi;

        sys::process_message("file_system_service", (uint64_t)&dat, sizeof(sys::file_system_service_protocol)).read();
        file_information ret = *fi;
        free(fi);
        return ret;
    }
    uint64_t read_file(uint64_t fid, uint8_t *data, uint64_t at, uint64_t length)
    {
        sys::file_system_service_protocol dat = {0};
        dat.request_type = FILE_READ;
        dat.read.at = at;
        dat.read.file_request_id = fid;
        dat.read.length = length;
        dat.read.target = data;
        uint64_t result = sys::process_message("file_system_service", (uint64_t)&dat, sizeof(sys::file_system_service_protocol)).read();
        return result;
    }
} // namespace sys
