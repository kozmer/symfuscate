#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Elf_Utils.h"
#include "Hashing.h"

/*!
 * @brief
 *  Function to find and resolve symbols in a shared object by their hash value.
 *  When symbols are found, they are stored in the provided function_info_t array
 *  through their resolved_ptr members.
 *
 * @param obj_path
 *  The path to the shared object file.
 * @param base_addr
 *  The base address of the shared object in memory.
 * @param functions
 *  An array of function_info_t structures containing information about the target functions.
 * @param num_functions
 *  The number of functions in the functions array.
 *
 * @return
 *  Always returns NULL. Function resolution status can be checked through the resolved_ptr 
 *  members of the functions array.
 */
void **find_hashed_symbols(const char *obj_path, ElfW(Addr) base_addr, function_info_t *functions, int num_functions)
{
    printf("[i] Searching in: %s\n", obj_path);

    //
    // Open the shared object
    //
    FILE *file = fopen(obj_path, "rb");
    if (!file)
    {
        printf("[!] Failed to open file: %s\n", obj_path);
        return NULL;
    }

    //
    // Read ELF header
    //
    ElfW(Ehdr) ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, file) != 1)
    {
        printf("[!] Failed to read ELF header from file: %s\n", obj_path);
        fclose(file);
        return NULL;
    }

    //
    // Read section headers
    //
    ElfW(Shdr) shdr[ehdr.e_shnum];
    fseek(file, ehdr.e_shoff, SEEK_SET);
    if (fread(shdr, sizeof(ElfW(Shdr)), ehdr.e_shnum, file) != ehdr.e_shnum)
    {
        printf("[!] Failed to read section headers from file: %s\n", obj_path);
        fclose(file);
        return NULL;
    }

    //
    // Find .dynsym and .dynstr sections
    //
    ElfW(Shdr) *dynsym_shdr = NULL, *dynstr_shdr = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++)
    {
        if (shdr[i].sh_type == SHT_DYNSYM)
        {
            dynsym_shdr = &shdr[i];
        }
        else if (shdr[i].sh_type == SHT_STRTAB && i != ehdr.e_shstrndx)
        {
            dynstr_shdr = &shdr[i];
        }
    }

    if (!dynsym_shdr || !dynstr_shdr)
    {
        printf("[!] Failed to find dynsym or dynstr sections\n");
        fclose(file);
        return NULL;
    }

    printf("[i] Found dynsym @ %p and dynstr @ %p\n", (void *)dynsym_shdr->sh_offset,
                (void *)dynstr_shdr->sh_offset);

    //
    // Read the .dynstr section
    //
    char *dynstr = malloc(dynstr_shdr->sh_size);
    fseek(file, dynstr_shdr->sh_offset, SEEK_SET);
    if (fread(dynstr, dynstr_shdr->sh_size, 1, file) != 1)
    {
        printf("[!] Failed to read .dynstr section from file: %s\n", obj_path);
        free(dynstr);
        fclose(file);
        return NULL;
    }

    //
    // Read the .dynsym section
    //
    ElfW(Sym) *dynsym = malloc(dynsym_shdr->sh_size);
    fseek(file, dynsym_shdr->sh_offset, SEEK_SET);
    if (fread(dynsym, dynsym_shdr->sh_size, 1, file) != 1)
    {
        printf("[!] Failed to read .dynsym section from file: %s\n", obj_path);
        free(dynstr);
        free(dynsym);
        fclose(file);
        return NULL;
    }

    //
    // Calculate the number of symbols
    //
    int num_symbols = dynsym_shdr->sh_size / sizeof(ElfW(Sym));
    printf("[i] Number of symbols: %d\n", num_symbols);

    //
    // Find the symbols by hashes
    //
    for (int i = 0; i < num_symbols; i++)
    {
        if (dynsym[i].st_name != 0)
        {
            const char *name = &dynstr[dynsym[i].st_name];
            
            for (int j = 0; j < num_functions; j++)
            {
                if (HASH(name) == functions[j].hash)
                {
                    void *resolved = (void *)(base_addr + dynsym[i].st_value);
                    *(functions[j].resolved_ptr) = resolved;
                    printf("[+] Found '%s' @ %p\n", name, resolved);
                    functions[j].name = name;  // Literally only have this to print in main :)
                }
            }
        }
    }

    free(dynstr);
    free(dynsym);
    fclose(file);

    return NULL;
}

/*!
 * @brief
 *  Callback function to be called for each shared object, in this case libc.so.
 *
 * @param info
 *  A pointer to the structure containing information about the shared object.
 * @param size
 *  The size of the structure pointed to by info.
 * @param data
 *  A pointer to the user data passed to dl_iterate_phdr.
 *
 * @return
 *  1 if the target function is resolved, 0 to continue iteration.
 */
int callback(struct dl_phdr_info *info, size_t size, void *data)
{
    void **callback_data = (void **)data;
    function_info_t *functions = callback_data[0];
    int num_functions = *(int *)(callback_data[1]);

    if (info->dlpi_name && info->dlpi_name[0]) {
        // Only interested in libc
        if (strstr(info->dlpi_name, "libc.so")) {
            void **resolved = find_hashed_symbols(info->dlpi_name, 
                                                info->dlpi_addr, 
                                                functions,
                                                num_functions);
            if (resolved) {
                free(resolved);
                return 1;
            }
        }
    }
    return 0;
}