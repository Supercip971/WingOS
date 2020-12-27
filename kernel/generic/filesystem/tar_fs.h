#pragma once
#include <filesystem/virt_fs.h>

enum tar_file_type_flag
{
    NORMAL_FILE = '0',
    HARD_LINK = '1',
    SYMBOLIC_LINK = '2',
    CHARACTER_DEVICE = '3',
    BLOCK_DEVICE = '4',
    DIRECTORY = '5',
    PIPE = '6'
};
#pragma pack(1)
struct tar_file_header
{
    char name[100];     /*   0 */
    char mode[8];       /* 100 */
    char uid[8];        /* 108 */
    char gid[8];        /* 116 */
    char size[12];      /* 124 */
    char mtime[12];     /* 136 */
    char chksum[8];     /* 148 */
    char typeflag;      /* 156 */
    char linkname[100]; /* 157 */
    char magic[6];      /* 257 */
    char version[2];    /* 263 */
    char uname[32];     /* 265 */
    char gname[32];     /* 297 */
    char devmajor[8];   /* 329 */
    char devminor[8];   /* 337 */
    char prefix[131];   /* 345 */
    char atime[12];     /* 476 */
    char ctime[12];     /* 488 */
} __attribute__((packed));
class tar_file : public virt_file
{
public:
    tar_file_header *header;
    tar_file_type_flag type;
    virtual const char *get_path() override;
    virtual uint64_t get_size() override;
};

class tar_fs : public virt_fs
{

protected:
    uint64_t file_count;
    static unsigned int file_size(tar_file_header *header)
    {

        printf("char %s \n", header->name);
        for (int i = 0; i < 12; i++)
        {
            printf("char %x == %c \n", i, header->size[i]);
        }
        unsigned int size = 0;
        unsigned int count = 1;

        for (uint32_t j = 11; j > 0; j--, count *= 8)
        {

            size += ((header->size[j - 1] - '0') * count);
        }

        return size;
    }

    tar_file *virt_file_list;
    tar_file_header *file_header_list[64];

public:
    tar_fs();
    virtual void init(uint64_t module_start, uint64_t module_end) override;
    bool is_valid_file(tar_file_header *header);
    uint64_t get_file_count();
    // read a file and redirect it to an address
    // return 0 when not found
    virtual uint64_t read_file(const char *path) override;

    // read a file and redirect it to an address wich point to a copy of the file
    // return 0 when not found
    virtual uint64_t read_file_copy(const char *path) override;

    // mainly not implemented
    virtual virt_file *get_file(const char *path) override;
};
