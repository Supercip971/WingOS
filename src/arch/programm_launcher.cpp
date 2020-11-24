#include <arch/mem/memory_manager.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <arch/programm_launcher.h>
#include <arch/smp.h>
#include <kernel.h>
#include <logging.h>
#include <utility.h>

uint8_t last_selected_cpu = 0;

void load_segment(process *pro, uintptr_t source, uint64_t size, uintptr_t dest, uint64_t destsize)
{
    uint64_t count = destsize / PAGE_SIZE;
    uintptr_t ndest = dest / PAGE_SIZE;
    ndest *= PAGE_SIZE;
    count++;
    source /= 4096;
    source *= 4096;
    for (uint64_t i = 0; i < count; i++)
    {
        uintptr_t target_virtual = ndest + i * PAGE_SIZE;
        map_page(source + PAGE_SIZE * i, target_virtual, 0x1 | 0x2 | 0x4);
    }
    add_thread_map(pro, source, ndest, count + 1);
    update_paging();
}

uint64_t get_elf_section_header(uint8_t *data, uint64_t code)
{
    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(data);

    Elf64_Shdr *p_entry = reinterpret_cast<Elf64_Shdr *>((uint64_t)data + programm_header->e_shoff);

    for (int table_entry = 0; table_entry < programm_header->e_shnum; table_entry++)
    {
        if (p_entry[table_entry].sh_type == code)
        {
            return table_entry;
        }
    }
    return -1;
}

char *read_elf_string_entry(uint8_t *data, uint64_t idx)
{
    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(data);
    Elf64_Shdr *p_entry = reinterpret_cast<Elf64_Shdr *>((uintptr_t)data + programm_header->e_shoff);

    uintptr_t result_offset = get_elf_section_header(data, SHT_STRTAB);
    result_offset = p_entry[result_offset].sh_offset;
    return (char *)(data + result_offset + (idx));
}

char *elf_to_readable_string(const char *string)
{
    char *temp = (char *)malloc(strlen(string) + 64);
    memzero(temp, (strlen(string) + 64));
    uint64_t temp_idx = 0;
    uint64_t cur_temp_idx = 0;
    bool first_d = false;
    bool should_continue = 0;
    if (strncmp(string, "_Z", 2) == 0)
    {
        while (isdigit(string[temp_idx]) == false)
        {
            if (string[temp_idx] == 'N')
            {
                first_d = true;
            }
            temp_idx++;
        }
        while (true)
        {

            uint64_t string_to_read_length = strtoint(string + temp_idx);

            while (isdigit(string[temp_idx]) == true)
            {
                temp_idx++;
            }
            //  memcpy(temp + cur_temp_idx, string + temp_idx, string_to_read_length);
            for (int i = 0; i < string_to_read_length; i++)
            {
                temp[cur_temp_idx++] = string[temp_idx++];
            }
            if (first_d == true)
            {
                temp[cur_temp_idx++] = ':';
                temp[cur_temp_idx++] = ':';
                first_d = false;
            }
            if (!isdigit(string[temp_idx]) || strlen(string) + 64 < cur_temp_idx)
            {
                break;
            }
        }
        return temp;
    }
    else
    {
        for (uint64_t i = 0; i < strlen(string) + 1; i++)
        {
            temp[i] = string[i];
        }
        return temp;
    }
}

void read_elf_section_header(uint8_t *data)
{

    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(data);

    Elf64_Shdr *p_entry = reinterpret_cast<Elf64_Shdr *>((uintptr_t)data + programm_header->e_shoff);
    for (int table_entry = 0; table_entry < programm_header->e_shnum; table_entry++)
    {
        log("prog launcher", LOG_INFO) << "detected sh entry : " << p_entry[table_entry].sh_type;
        if (p_entry[table_entry].sh_type == SHT_SYMTAB)
        {
            Elf64_Sym *entry = reinterpret_cast<Elf64_Sym *>(data + p_entry[table_entry].sh_offset);

            for (int64_t sym_entry_idx = 0; sym_entry_idx < p_entry[table_entry].sh_size / sizeof(Elf64_Sym); sym_entry_idx++)
            {
                log("prog launcher", LOG_INFO) << "[sht symtab]" << sym_entry_idx
                                               << " name : " << entry[sym_entry_idx].st_name;

                if (entry[sym_entry_idx].st_name != 0)
                {
                    char *res = elf_to_readable_string((read_elf_string_entry(data, entry[sym_entry_idx].st_name)));
                    log("prog launcher", LOG_INFO) << "final name : " << res;
                    free(res);
                }
            }
        }
    }
}

bool valid_elf_entry(Elf64_Ehdr *entry)
{
    if (entry->e_ident[0] != 0x7f ||
        entry->e_ident[1] != 'E' ||
        entry->e_ident[2] != 'L' ||
        entry->e_ident[3] != 'F')
    {
        return false;
    }

    if (entry->e_ident[4] != ELFCLASS64)
    {
        return false;
    }
    return true;
}

uint64_t get_programm_cpu()
{
    last_selected_cpu++;
    if (last_selected_cpu > smp::the()->processor_count)
    {
        last_selected_cpu = 0;
    }
    return last_selected_cpu;
}

void elf64_load_programm_segment(Elf64_Phdr *entry, uint8_t *programm_code, process *target)
{
    char *temp_copy = (char *)malloc(entry->p_memsz + 4096);
    memzero(temp_copy, entry->p_memsz + 4096);
    memcpy(temp_copy, (char *)((uintptr_t)programm_code + entry->p_offset), entry->p_filesz);
    load_segment(target, (uintptr_t)programm_code + entry->p_offset, entry->p_filesz, entry->p_vaddr, entry->p_memsz);
    char *p_entry_data = (char *)entry->p_vaddr;
    memcpy(p_entry_data, temp_copy, entry->p_memsz);
}

void elf64_load_entry(Elf64_Phdr *entry, uint8_t *programm_code, process *target)
{
    if (entry->p_type == PT_LOAD)
    {
        elf64_load_programm_segment(entry, programm_code, target);
    }
    else
    {

        log("prog launcher", LOG_ERROR) << "not supported entry type : " << entry->p_type;
    }
}

void launch_programm(const char *path, file_system *file_sys)
{

    log("prog launcher", LOG_DEBUG) << "launching programm : " << path;
    uint8_t *programm_code = file_sys->read_file(path);

    if (programm_code == nullptr)
    {
        return;
    }

    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(programm_code);
    if (!valid_elf_entry(programm_header))
    {
        log("prog launcher", LOG_ERROR) << "not valid elf64 entry";
        return;
    }

    uint64_t cpu_programm = get_programm_cpu();

    log("prog launcher", LOG_INFO) << "elf programm cpu : " << cpu_programm;

    process *to_launch = init_process((func)programm_header->e_entry, false, path, true, cpu_programm);

    Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)programm_code + programm_header->e_phoff);

    for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++, p_entry += programm_header->e_phentsize)
    {
        elf64_load_entry(p_entry, programm_code, to_launch);
    }

    to_launch->current_process_state = process_state::PROCESS_WAITING;
}
