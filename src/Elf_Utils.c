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
 *  Function to find symbols in a shared object by their hash value.
 *
 * @param obj_path
 *  The path to the shared object file.
 * @param base_addr
 *  The base address of the shared object in memory.
 * @param target_hashes
 *  An array of target hash values to match against the symbol names.
 * @param num_hashes
 *  The number of hash values in the target_hashes array.
 *
 * @return
 *  An array of pointers to the resolved functions, or NULL if not found.
 */
void **find_hashed_symbols(const char *obj_path, ElfW(Addr) base_addr, uint32_t *target_hashes, int num_hashes)
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

    void **resolved_functions = malloc(num_hashes * sizeof(void *));
    for (int i = 0; i < num_hashes; i++)
    {
        resolved_functions[i] = NULL;
    }

    //
    // Find the symbols by hashes
    //
    for (int i = 0; i < num_symbols; i++)
    {
        if (dynsym[i].st_name != 0)
        {
            const char *name = &dynstr[dynsym[i].st_name];
            uint32_t name_hash = HASH(name);
            // printf("[i] Checking symbol: %s (hash: %u)\n", name, name_hash);

            for (int j = 0; j < num_hashes; j++)
            {
                if (name_hash == target_hashes[j])
                {
                    resolved_functions[j] = (void *)(base_addr + dynsym[i].st_value);
                    printf("[+] Found '%s' in %s @ %p\n", name, obj_path, resolved_functions[j]);
                }
            }
        }
    }

    free(dynstr);
    free(dynsym);
    fclose(file);

    return resolved_functions;
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
    void **resolved_functions = callback_data[0];
    uint32_t *target_hashes = callback_data[1];
    int num_hashes = *(int *)(callback_data[2]);

    if (info->dlpi_name && info->dlpi_name[0])
    {
        // Only interested in libc
        if (strstr(info->dlpi_name, "libc.so"))
        {
            void **functions = find_hashed_symbols(info->dlpi_name, info->dlpi_addr, target_hashes, num_hashes);
            if (functions)
            {
                for (int i = 0; i < num_hashes; i++)
                {
                    resolved_functions[i] = functions[i];
                }
                free(functions);
                return 1;
            }
        }
    }

    return 0;
}