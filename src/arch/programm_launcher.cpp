#include <arch/mem/liballoc.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <arch/programm_launcher.h>
#include <arch/smp.h>
#include <kernel.h>
#include <loggging.h>
#include <utility.h>
void load_segment(process *pro, uint64_t source, uint64_t size, uint64_t dest, uint64_t destsize)
{
    uint64_t count = destsize / PAGE_SIZE;
    uint64_t ndest = dest / PAGE_SIZE;
    ndest *= PAGE_SIZE;
    count++;
    source /= 4096;
    source *= 4096;
    for (uint64_t i = 0; i < count; i++)
    {
        uint64_t target_virtual = ndest + i * PAGE_SIZE;
        map_page(source + PAGE_SIZE * i, target_virtual, 0x1 | 0x2 | 0x4);
    }
    add_thread_map(pro, (uint64_t)source, ndest, count + 1);
    update_paging();
}
uint8_t last_selected_cpu = 0;
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
        last_selected_cpu++;
        if (last_selected_cpu > smp::the()->processor_count)
        {
            last_selected_cpu = 0;
        }

        log("prog launcher", LOG_INFO) << "elf programm cpu : " << last_selected_cpu;
        process *to_launch = init_process((func)programm_header->e_entry, false, path, true, last_selected_cpu);

        Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uint64_t)programm_code + programm_header->e_phoff);
        for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++, p_entry += programm_header->e_phentsize)
        {

            if (p_entry->p_type == PT_LOAD)
            {

                char *temp_copy = (char *)malloc(p_entry->p_filesz + 4096);
                memcpy(temp_copy, (char *)((uint64_t)programm_code + p_entry->p_offset), p_entry->p_filesz);
                load_segment(to_launch, (uint64_t)programm_code + p_entry->p_offset, p_entry->p_filesz, p_entry->p_vaddr, p_entry->p_memsz);
                char *p_entry_data = (char *)p_entry->p_vaddr;
                for (int i = 0; i < p_entry->p_filesz; i++)
                {
                    p_entry_data[i] = temp_copy[i];
                }
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
