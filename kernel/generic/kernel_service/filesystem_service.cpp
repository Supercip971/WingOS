#include <filesystem/file_system.h>
#include <kernel.h>
#include <kernel_service/filesystem_service.h>
#include <logging.h>
#include <process.h>
#include <utility.h>
#include <utils/liballoc.h>
struct fss_file_handle
{
    uint64_t id;
    bool opened;
    bool free_to_use;
    char file_path[255];
    uint64_t opened_by_process;
};

uint64_t next_fss_uid = 1;       // 0 = null
fss_file_handle handle_list[64]; // max 64 file handle at the same time :o

fss_file_handle *get_file(uint64_t id, uint64_t pid)
{
    for (int i = 0; i < 64; i++)
    {
        if (handle_list[i].id == id)
        {
            if (handle_list[i].opened_by_process == pid)
            {

                return &handle_list[i];
            }
        }
    }
    return nullptr;
}

uint64_t file_open(process_message *msg)
{
    file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

    for (int i = 0; i < 64; i++)
    {
        if (handle_list[i].free_to_use == true)
        {
            handle_list[i].free_to_use = false;
            handle_list[i].opened = true;

            handle_list[i].id = next_fss_uid++;
            handle_list[i].opened_by_process = msg->from_pid;
            memcpy(handle_list[i].file_path, prot->open.file_directory, strlen((char *)prot->open.file_directory) + 1);
            return handle_list[i].id;
        }
    }
    return 0;
}

uint64_t file_close(process_message *msg)
{
    file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

    fss_file_handle *target = get_file(prot->close.file_request_id, msg->from_pid);
    target->opened = false;
    target->id = 0;
    target->free_to_use = true;
    return 0;
}

uint64_t file_read(process_message *msg)
{

    file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

    fss_file_handle *target = get_file(prot->read.file_request_id, msg->from_pid);
    if (target == nullptr)
    {
        log("file system service", LOG_INFO) << "no file opened :^( ) ";
        return -1;
    }

    char *path = (char *)target->file_path;
    log("file system service", LOG_INFO) << "reading " << path;

    size_t readed_length = main_fs_system::the()->main_fs()->read_file(path, prot->read.at, prot->read.length, prot->read.target);

    log("file system service", LOG_INFO) << "readed " << path << "for " << readed_length << "cursor" << prot->read.at;

    log("file system service", LOG_INFO) << "readed " << (char *)prot->read.target;
    return readed_length;
}

uint64_t file_get_information(process_message *msg)
{
    file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

    file_information *target = prot->info.target;
    fss_file_handle *file = get_file(prot->info.file_request_id, msg->from_pid);

    target->size = main_fs_system::the()->main_fs()->get_file_length(file->file_path);
    return 1;
}

void file_system_service()
{
    for (int i = 0; i < 64; i++)
    {
        handle_list[i].free_to_use = true;
    }
    log("file system service", LOG_INFO) << "loaded file system service";
    set_on_request_service(true);
    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            set_on_request_service(false);
            file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

            switch (prot->request_type)
            {
            case FILE_OPEN:
                msg->response = file_open(msg);
                break;
            case FILE_CLOSE:
                msg->response = file_close(msg);
                break;
            case FILE_READ:
                msg->response = file_read(msg);
                break;
            case GET_FILE_INFO:
                msg->response = file_get_information(msg);
                break;
            default:
                log("file system service", LOG_ERROR) << "not handled request id" << prot->request_type;
                msg->response = -1;
                break;
            }
            if (msg->response == 0)
            {
                msg->response = -1;
            }

            msg->has_been_readed = true;
            set_on_request_service(true);
        }
        else if (msg == 0)
        {
        }
    }
}
