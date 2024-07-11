#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <elf.h>
#include <stdint.h>

int callback(struct dl_phdr_info *info, size_t size, void *data);

#endif // ELF_UTILS_H