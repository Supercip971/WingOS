#include <arch/mem/liballoc.h>
#include <arch/mem/virtual.h>
#include <arch/programm_launcher.h>
#include <kernel.h>
#include <loggging.h>
#include <utility.h>
void load_segment(process *pro, uint64_t source, uint64_t size, uint64_t dest, uint64_t destsize)
{
    uint64_t count = destsize / PAGE_SIZE;

    uint64_t ndest = dest / PAGE_SIZE;
    ndest *= PAGE_SIZE;

    if (ndest < dest)
    {
        count++;
    }

    source /= 4096;
    source *= 4096;

    for (int i = 0; i < count; i++)
    {
        uint64_t target_virtual = ndest + i * PAGE_SIZE;
        virt_map(target_virtual, source + PAGE_SIZE * i, 0x1 | 0x2 | 0x4);
    }

    add_thread_map(pro, ndest, (uint64_t)source, count);
    update_paging();
}
void launch_programm(const char *path, echfs *file_sys)
{

    log("prog launcher", LOG_DEBUG) << "launching programm : " << path;
    uint8_t *programm_code = file_sys->ech_read_file(path);
    if (programm_code == nullptr)
    {
        return;
    }
    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(programm_code);
    if (programm_header->e_ident[0] == 0x7f &&
        programm_header->e_ident[1] == 'E' &&
        programm_header->e_ident[2] == 'L' &&
        programm_header->e_ident[3] == 'F')
    {

        log("prog launcher", LOG_INFO) << "valid elf programm ";
        if (programm_header->e_ident[4] != ELFCLASS64)
        {

            log("prog launcher", LOG_ERROR) << "is not 64bit programm ";
        }
        log("prog launcher", LOG_INFO) << "elf programm entry count" << programm_header->e_phnum;
        process *to_launch = init_process((func)programm_header->e_entry, false);
        Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uint64_t)programm_code + programm_header->e_phoff);
        for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++, p_entry += programm_header->e_phentsize)
        {

            if (p_entry->p_type == PT_LOAD)
            {
                load_segment(to_launch, (uint64_t)programm_code + p_entry->p_offset, p_entry->p_filesz, p_entry->p_vaddr, p_entry->p_memsz);

                memcpy((char *)p_entry->p_vaddr, (char *)(programm_code + p_entry->p_offset), p_entry->p_filesz);
                //       printf("loading valid elf entry \n");

                //     load_segment(p_entry->p_offset, p_entry->p_filesz, p_entry->p_vaddr, p_entry->p_memsz);

                //   printf("loading valid elf entry OK \n");
            }
            else
            {
                log("prog launcher", LOG_ERROR) << "not supported entry type : " << p_entry->p_type;
            }
        }

        to_launch->current_process_state = process_state::PROCESS_WAITING;
    }
    else
    {

        log("prog launcher", LOG_ERROR) << "not valid elf programm ";
        return;
    }
}
