#include <arch/process.h>
#include <filesystem/file_system.h>
#include <kernel.h>
#include <kernel_service/filesystem_service.h>
#include <logging.h>
#include <utility.h>
struct fss_file_handle
{
    uint64_t id;
    bool opened;
    bool free_to_use;
    char file_path[255];
    uint64_t opened_by_process;
    uint8_t *data;
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

            handle_list[i].data = nullptr;
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
    if (target->data != nullptr)
    {
        free(target->data);
    }
    target->data = nullptr;
    target->free_to_use = true;
    return 0;
}
uint64_t file_read(process_message *msg)
{

    file_system_service_protocol *prot = reinterpret_cast<file_system_service_protocol *>(msg->content_address);

    fss_file_handle *target = get_file(prot->read.file_request_id, msg->from_pid);
    if (target == nullptr)
    {
        log("file system service", LOG_INFO) << "no file founded :^( ) ";
    }
    uint64_t readed_length = 0;

    char *path = (char *)target->file_path;
    log("file system service", LOG_INFO) << "reading " << path;

    if (target->data == nullptr)
    {
        log("file system service", LOG_INFO) << "reading for first time" << path;
        target->data = main_fs_system::the()->main_fs()->read_file(path);
    }
    else
    {
        log("file system service", LOG_INFO) << "file has already been readed";
    }
    uint64_t file_lenth = main_fs_system::the()->main_fs()->get_file_length(path);
    if (prot->read.at > file_lenth)
    {
        log("file system service", LOG_ERROR) << "trying to read outside of data";
        return 0;
    }
    if (prot->read.length + prot->read.at > main_fs_system::the()->main_fs()->get_file_length(path))
    {
        readed_length = main_fs_system::the()->main_fs()->get_file_length(path) - prot->read.at;
    }
    else
    {
        readed_length = prot->read.length;
    }
    memcpy((void *)((uint64_t)prot->read.target), target->data + prot->read.at, prot->read.length);

    log("file system service", LOG_INFO) << "readed " << path;
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

            set_on_request_service(false);
            if (prot->request_type == file_system_service_request::FILE_OPEN)
            {
                msg->response = file_open(msg);
            }
            else if (prot->request_type == file_system_service_request::FILE_CLOSE)
            {
                msg->response = file_close(msg);
            }
            else if (prot->request_type == file_system_service_request::FILE_READ)
            {
                msg->response = file_read(msg); // temp fix
            }
            else if (prot->request_type == file_system_service_request::GET_FILE_INFO)
            {
                msg->response = file_get_information(msg);
            }
            else
            {
                log("file system service", LOG_ERROR) << "not handled request id" << prot->request_type;
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
