#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <elf.h>
#include <stdint.h>
#include "Function_Resolver.h"

int callback(struct dl_phdr_info *info, size_t size, void *data);
void **find_hashed_symbols(const char *obj_path, 
                          ElfW(Addr) base_addr, 
                          function_info_t *functions,
                          int num_functions);

#endif // ELF_UTILS_H