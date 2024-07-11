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
 *  Main function to initialize and perform symbol resolution.
 *
 *  This function resolves the 'system' and 'execve' functions by their hash values and then calls them.
 *
 * @return
 *  0 on success, 1 on failure
 */
int main()
{

    //
    // Function pointer types
    //
    typedef int (*system_func_t)(const char *);
    typedef int (*execve_func_t)(const char *, char *const[], char *const[]);

    //
    // Define hash values for functions, in this case 'system' & 'execve'.
    // [0] == system, [1] == execve
    //
    uint32_t function_hashes[] = {2227611796, 1678009295};
    int num_hashes = sizeof(function_hashes) / sizeof(function_hashes[0]);

    //
    // Alloc memory for resolved function pointers
    //
    void **resolved_functions = malloc(num_hashes * sizeof(void *));
    for (int i = 0; i < num_hashes; i++)
    {
        resolved_functions[i] = NULL;
    }

    //
    // Prep callback data
    //
    void *callback_data[] = {resolved_functions, function_hashes, &num_hashes};

    //
    // Iterate over shared objects
    //
    dl_iterate_phdr(callback, callback_data);

    //
    // Check all functions resolve
    //
    for (int i = 0; i < num_hashes; i++)
    {
        if (resolved_functions[i])
        {
            printf("[+] Resolved function @ i[%d]: %p\n", i, resolved_functions[i]);
        }
        else
        {
            printf("[!] Failed to resolve function @ i[%d]\n", i);
            free(resolved_functions);
            printf("[!] Exiting...\n");
            exit(1);
        }
    }

    //
    // If all functions resolved successfully, goto to the resolved section
    //
    goto resolved;

    //
    // Cast and use the resolved functions
    //
    resolved: {
        system_func_t _system = (system_func_t)resolved_functions[0];
        _system("uname");

        execve_func_t _execve = (execve_func_t)resolved_functions[1];
        char *const argv[] = {"/bin/ls", NULL};
        char *const envp[] = {NULL};
        _execve("/bin/ls", argv, envp);
    }

    free(resolved_functions);

    return 0;
}