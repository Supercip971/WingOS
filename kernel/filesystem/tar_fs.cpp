
#include <arch/mem/memory_manager.h>
#include <com.h>
#include <filesystem/tar_fs.h>
#include <utility.h>
const char *tar_file::get_path()
{
    return nullptr;
}

uint64_t tar_file::get_size()
{
    return -1;
}

tar_fs::tar_fs()
{
}

void tar_fs::init(uint64_t module_start, uint64_t module_end)
{
    fs_module_addr = module_start;
    fs_module_end = module_end;
    printf("loading tarfs, start %x, end %x, virtual disk lenght %x \n", module_start, module_end, module_end - module_start);
    printf("loading tarfs file count = %x \n", get_file_count());
    for (uint64_t i = 0; i < file_count + 2; i++)
    {
        file_header_list[i] = 0;
    }
    printf("parsing tarfs");
    uint64_t save_addr = module_start;
    for (uint32_t i = 0; i <= file_count; i++)
    {
        printf(" \n ======= file ======= \n ");
        tar_file_header *header = (tar_file_header *)(save_addr);

        if (header->name[0] == '\0')
        {
            break;
        }
        else
        {
            printf("getting file name %s data... \n", header->name);
        }

        uint32_t size = file_size(header);

        file_header_list[i] = header;

        save_addr += (((size + 511) / 512) + 1) * 512;

        if (!is_valid_file(header) || save_addr > fs_module_end)
        {
            break;
        }
        printf("file information : \n");

        if (file_header_list[i]->typeflag == DIRECTORY)
        {
            printf("type : directory \n");
            if (size != 0)
            {
                printf("invalid directory lenght");
            }
            printf("name : %s \n", file_header_list[i]->name);
        }
        else if (file_header_list[i]->typeflag == NORMAL_FILE)
        {
            printf("type : file \n");
            printf("size : %x \n", size);
            printf("name : %s \n", file_header_list[i]->name);
        }
        else
        {
            printf("not supported file type : %c", file_header_list[i]->typeflag);
        }
    }
}

bool tar_fs::is_valid_file(tar_file_header *header)
{
    if (strncmp(header->magic, "ustar", 5))
    {
        return true;
    }
    return false;
}
uint64_t tar_fs::get_file_count()
{
    file_count = 0;
    printf("getting file count .... \n");
    uint64_t save_addr = fs_module_addr;
    for (file_count = 0;; file_count++)
    {
        tar_file_header *header = (tar_file_header *)(save_addr);

        uint32_t size = file_size(header);
        printf("checking file size %s \n", header->size);
        printf("found file of size %x, at %x \n", size, (uint64_t)header);
        save_addr += (((size + 511) / 512) + 1) * 512;
        if (!is_valid_file(header) || save_addr > fs_module_end)
        {
            break;
        }
    }
    return file_count;
}
uint64_t tar_fs::read_file(const char *path)
{
    return 0;
}
uint64_t tar_fs::read_file_copy(const char *path)
{
    return 0;
}

virt_file *tar_fs::get_file(const char *path)
{
    return nullptr;
}
