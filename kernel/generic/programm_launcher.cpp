#include <process_context.h>

#include <arch.h>
#include <ctypes.h>
#include <elf_gnu_structure.h>
#include <filesystem/file_system.h>
#include <kernel.h>
#include <logging.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>
#include <utils/lock.h>
#include <utils/memory/liballoc.h>
#include <utils/sys/programm_exec_info.h>
uint8_t last_selected_cpu = 0;
utils::lock_type prg_launch;

struct module_info
{
    uintptr_t init;
    uintptr_t fini;
};

void load_segment(process *pro, uintptr_t source, uint64_t size, uintptr_t dest, uint64_t destsize)
{
    uint64_t count = destsize / PAGE_SIZE;
    uintptr_t ndest = ALIGN_DOWN(dest, PAGE_SIZE);
    source = ALIGN_DOWN(source, PAGE_SIZE);
    count++;
    add_thread_map(pro, source, ndest, count + 1);
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
            // FIXME: just implement strtoint in libc
            // uint64_t string_to_read_length = strtoint(string + temp_idx);
            uint64_t string_to_read_length = 0;
            while (isdigit(string[temp_idx]) == true)
            {
                temp_idx++;
            }
            //  memcpy(temp + cur_temp_idx, string + temp_idx, string_to_read_length);
            for (uint64_t i = 0; i < string_to_read_length; i++)
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
        log("prog launcher", LOG_INFO, "detected sh entry: {}", p_entry[table_entry].sh_type);
        if (p_entry[table_entry].sh_type == SHT_SYMTAB)
        {
            Elf64_Sym *entry = reinterpret_cast<Elf64_Sym *>(data + p_entry[table_entry].sh_offset);

            for (uint64_t sym_entry_idx = 0; sym_entry_idx < p_entry[table_entry].sh_size / sizeof(Elf64_Sym); sym_entry_idx++)
            {
                log("prog launcher", LOG_INFO, "[sht symtab] {} name: {}", sym_entry_idx, entry[sym_entry_idx].st_name);

                if (entry[sym_entry_idx].st_name != 0)
                {
                    char *res = elf_to_readable_string((read_elf_string_entry(data, entry[sym_entry_idx].st_name)));
                    log("prog launcher", LOG_INFO, "final name: {}", res);
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

void elf64_load_programm_segment(Elf64_Phdr *entry, uint8_t *programm_code, process *target)
{

    size_t max_size = utils::max(entry->p_memsz, entry->p_filesz);
    char *temp_copy = (char *)get_mem_addr(pmm_alloc_zero((max_size + PAGE_SIZE) / PAGE_SIZE));
    memzero(temp_copy, max_size);
    memcpy(temp_copy, (char *)((uintptr_t)programm_code + entry->p_offset), entry->p_filesz);
    load_segment(target, (uintptr_t)get_rmem_addr(temp_copy), entry->p_filesz, entry->p_vaddr, entry->p_memsz);
}

void elf64_load_module_segment(Elf64_Phdr *entry, uint8_t *programm_code, uint8_t *target)
{
    size_t max_size = utils::max(entry->p_memsz, entry->p_filesz);
    memset(target + entry->p_vaddr, 0, max_size);
    memcpy(target + entry->p_vaddr, programm_code + entry->p_offset, entry->p_filesz);
}

void elf64_load_module_dyn_info(Elf64_Phdr *entry, uint8_t *programm_code, uint8_t *target, module_info *info)
{
    Elf64_Dyn *targ = (Elf64_Dyn *)(programm_code + entry->p_offset);

    for (size_t i = 0; i < (entry->p_filesz / sizeof(Elf64_Dyn)); i += 1)
    {
        // todo: implement dyn reading
        log("prog launcher", LOG_INFO, "dyn type: {} value: {}", targ[i].d_tag, targ[i].d_un.d_val);
        if (targ[i].d_tag == DT_NULL) // end of dyn section
        {
            return;
        }
        else if (targ[i].d_tag == DT_INIT)
        {
            info->init = targ[i].d_un.d_ptr;
        }
        else if (targ[i].d_tag == DT_FINI)
        {
            info->fini = targ[i].d_un.d_ptr;
        }
    }
}

void elf64_reloc_relative(Elf64_Rela *table, uint8_t *target, uintptr_t &offset)
{
    offset = (uintptr_t)target;
    offset += table->r_addend;
    memcpy(target + table->r_offset, &offset, sizeof(uintptr_t));
}
void elf64_load_module_relocation_addens(Elf64_Shdr *section_header, uint8_t *code, uint8_t *target, uintptr_t &offset)
{

    Elf64_Rela *table = (Elf64_Rela *)(section_header->sh_addr + (uintptr_t)code);
    while ((uintptr_t)table - (section_header->sh_addr + (uintptr_t)code) < section_header->sh_size)
    {
        uint32_t type = ELF64_R_TYPE(table->r_info);

        // todo: add more relocation support
        if (type == R_X86_64_RELATIVE)
        {
            elf64_reloc_relative(table, target, offset);
        }
        else
        {
            log("prog launcher", LOG_WARNING, "unknown reloc type: {}", type);
        }
        table++;
    }
}
void elf64_load_module_relocation(Elf64_Ehdr *header, uint8_t *code, uint8_t *target)
{

    for (uintptr_t x = 0; x < header->e_shentsize * header->e_shnum; x += header->e_shentsize)
    {
        Elf64_Shdr *section_header = (Elf64_Shdr *)(code + header->e_shoff + x);

        if (section_header->sh_type == SHT_RELA)
        {
            elf64_load_module_relocation_addens(section_header, code, target, x);
        }
        else if (section_header->sh_type != SHT_NULL)
        {
            log("prog launcher", LOG_WARNING, "section header not supported entry: {}", section_header->sh_type);
        }
    }
}

void elf64_load_entry(Elf64_Phdr *entry, uint8_t *programm_code, process *target)
{
    if (entry->p_type == PT_LOAD)
    {
        elf64_load_programm_segment(entry, programm_code, target);
    }
    else if (entry->p_type == PT_NULL)
    {
    }
    else
    {

        log("prog launcher", LOG_ERROR, "not supported entry type: {}", entry->p_type);
    }
}

void elf64_load_module_entry(Elf64_Phdr *entry, uint8_t *programm_code, uint8_t *target, module_info *info)
{
    if (entry->p_type == PT_LOAD)
    {
        elf64_load_module_segment(entry, programm_code, target);
    }
    else if (entry->p_type == PT_DYNAMIC)
    {

        elf64_load_module_dyn_info(entry, programm_code, target, info);
    }
    else if (entry->p_type == PT_NULL || entry->p_type == PT_PHDR)
    {
    }
    else
    {

        log("prog launcher (module)", LOG_ERROR, "not supported entry type: {}", entry->p_type);
    }
}
// FIXME: the code of launch_programm launch_programm_usr & launch_module is ultra repetitive, make this code better and use function
uint64_t launch_programm(const char *path, file_system *file_sys, int argc, const char **argv)
{
    utils::context_lock locker(file_sys->fs_lock);
    lock_process();
    log("prog launcher", LOG_DEBUG, "launching programm: {}", path);
    uint8_t *programm_code = file_sys->read_file(path);

    if (programm_code == nullptr)
    {
        unlock_process();
        return -1;
    }

    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(programm_code);
    if (!valid_elf_entry(programm_header))
    {
        log("prog launcher", LOG_ERROR, "not valid elf64 entry");
        unlock_process();
        return -1;
    }
    char **end_argv = (char **)malloc(sizeof(char *) * argc + 1);
    end_argv[0] = (char *)malloc(strlen(path) + 1);
    memcpy(end_argv[0], path, strlen(path) + 1);
    for (int i = 0; i < argc; i++)
    {

        end_argv[i + 1] = (char *)malloc(strlen(argv[i]) + 1);
        memcpy(end_argv[i + 1], argv[i], strlen(argv[i]) + 1);
    }
    process *to_launch = init_process((func)programm_header->e_entry, false, path, true, AUTO_SELECT_CPU, argc + 1, end_argv);

    Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)programm_code + programm_header->e_phoff);

    for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++)
    {
        elf64_load_entry(p_entry, programm_code, to_launch);
        p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)p_entry + programm_header->e_phentsize);
    }

    to_launch->set_state(process_state::PROCESS_WAITING);

    unlock_process();
    return to_launch->get_pid();
}
size_t launch_programm_usr(programm_exec_info *info)
{
    lock_process();
    log("prog launcher", LOG_DEBUG, "starting {} with name {}", info->path, info->name);
    uint8_t *programm_code = main_fs_system::the()->main_fs()->read_file(info->path);

    if (programm_code == nullptr)
    {
        unlock_process();
        return -1;
    }

    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(programm_code);
    if (!valid_elf_entry(programm_header))
    {
        log("prog launcher", LOG_ERROR, "not valid elf64 entry");
        unlock_process();
        return -1;
    }
    char **end_argv = (char **)malloc(sizeof(char *) * (info->argc + 1));
    end_argv[0] = (char *)malloc(strlen(info->path) + 1);
    memcpy(end_argv[0], info->path, strlen(info->path) + 1);
    for (int i = 0; i < info->argc; i++)
    {

        end_argv[i + 1] = (char *)malloc(strlen(info->argv[i]) + 1);
        memcpy(end_argv[i + 1], info->argv[i], strlen(info->argv[i]) + 1);
    }
    process *to_launch = init_process((func)programm_header->e_entry, false, info->path, true, AUTO_SELECT_CPU, info->argc + 1, end_argv);

    Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)programm_code + programm_header->e_phoff);

    for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++, p_entry += programm_header->e_phentsize)
    {
        elf64_load_entry(p_entry, programm_code, to_launch);
    }
    if (info->name != nullptr)
    {
        to_launch->rename(info->name);
    }
    to_launch->set_state(process_state::PROCESS_WAITING);
    unlock_process();
    return to_launch->get_pid();
}
// find the end of the address space of a PIC
size_t get_module_table_max_addr(Elf64_Phdr *p_entry, Elf64_Ehdr *programm_header)
{
    size_t current_max = 0;
    for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++)
    {
        size_t max_size = utils::max(p_entry->p_memsz, p_entry->p_filesz);
        if (max_size + p_entry->p_vaddr > current_max)
        {
            current_max = max_size + p_entry->p_vaddr + 16;
        }
        p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)p_entry + programm_header->e_phentsize);
    }
    return current_max;
}
uint64_t launch_module(const char *path, file_system *file_sys, int argc, const char **argv)
{
    utils::context_lock locker(file_sys->fs_lock);
    lock_process();
    log("prog launcher", LOG_DEBUG, "launching module: {}", path);
    uint8_t *programm_code = file_sys->read_file(path);

    if (programm_code == nullptr)
    {
        unlock_process();
        return -1;
    }

    Elf64_Ehdr *programm_header = reinterpret_cast<Elf64_Ehdr *>(programm_code);
    if (!valid_elf_entry(programm_header))
    {
        log("prog launcher", LOG_ERROR, "not valid elf64 entry");
        for (int i = 0; i < 5; i++)
        {
            log("prog launcher", LOG_WARNING, "elf byte: {} = {} && {}", i, programm_header->e_ident[i], programm_code[i]);
        }
        unlock_process();
        return -1;
    }
    char **end_argv = (char **)malloc(sizeof(char *) * argc + 1);
    end_argv[0] = (char *)malloc(strlen(path) + 1);
    memcpy(end_argv[0], path, strlen(path) + 1);
    for (int i = 0; i < argc; i++)
    {

        end_argv[i + 1] = (char *)malloc(strlen(argv[i]) + 1);
        memcpy(end_argv[i + 1], argv[i], strlen(argv[i]) + 1);
    }
    module_info info = {0, 0};

    Elf64_Phdr *p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)programm_code + programm_header->e_phoff);
    uint8_t *target_end_code = (uint8_t *)malloc((get_module_table_max_addr(p_entry, programm_header) + 128));
    log("prog launcher", LOG_DEBUG, "launching module: {} offset: {}", path, (uintptr_t)(target_end_code));
    for (int table_entry = 0; table_entry < programm_header->e_phnum; table_entry++)
    {
        elf64_load_module_entry(p_entry, programm_code, target_end_code, &info);
        p_entry = reinterpret_cast<Elf64_Phdr *>((uintptr_t)p_entry + programm_header->e_phentsize);
    }
    process *to_launch = init_process((func)((uintptr_t)target_end_code + info.init), false, path, false, AUTO_SELECT_CPU, argc + 1, end_argv);

    elf64_load_module_relocation(programm_header, programm_code, target_end_code);
    to_launch->set_module(true);
    to_launch->set_state(process_state::PROCESS_WAITING);

    unlock_process();
    return to_launch->get_pid();
}
