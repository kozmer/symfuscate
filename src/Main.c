#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Elf_Utils.h"
#include "Hashing.h"
#include "Function_Resolver.h"

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
    resolved_functions_t resolved = {0};

    function_info_t functions[] = {
        {NULL, 2227611796, (void **)&resolved.system},
        {NULL, 1678009295, (void **)&resolved.execve}
    };
    
    int num_functions = sizeof(functions) / sizeof(functions[0]);

    //
    // Prep callback data
    //
    void *callback_data[] = {functions, &num_functions};

    // 
    // Iterate over shared objects
    //
    dl_iterate_phdr(callback, callback_data);

    //
    // Check all functions resolved
    //
    for (int i = 0; i < num_functions; i++) {
        if (*functions[i].resolved_ptr) {
            printf("[+] Resolved '%s' @ %p\n", 
                   functions[i].name,
                   *functions[i].resolved_ptr);
        } else {
            printf("[!] Failed to resolve function (hash: %u)\n",
                   functions[i].hash);
            printf("[!] Exiting...\n");
            exit(1);
        }
    }

    //
    // Use the resolved functions
    //
    resolved.system("uname");

    char *const argv[] = {"/bin/ls", NULL};
    char *const envp[] = {NULL};
    resolved.execve("/bin/ls", argv, envp);

    return 0;
}