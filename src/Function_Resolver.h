#ifndef FUNCTION_RESOLVER_H
#define FUNCTION_RESOLVER_H

#include <stdint.h>

//
// Function pointer types
//
typedef int (*system_func_t)(const char *);
typedef int (*execve_func_t)(const char *, char *const[], char *const[]);

//
// Struct to hold function information
//
typedef struct {
    const char *name;
    uint32_t hash;
    void **resolved_ptr;
} function_info_t;

//
// Struct to hold resolved functions
//
typedef struct {
    system_func_t system;
    execve_func_t execve;
} resolved_functions_t;

#endif // FUNCTION_RESOLVER_H