#include "cabx_i.h"
#include <stdlib.h>
#include <stddef.h>


/**
 * duplicate string
 */
char*
cabx_i_str_dup_0(
    const char* src,
    size_t length)
{
    char* result;
    result = (char*)cabx_i_mem_alloc(length);
    if (result) {
        memcpy(result, src, length);
    }
    return result;
}

/**
 * duplicate string
 */
char*
cabx_i_str_dup(
    const char* src)
{
    char* result;
    result = NULL;
    if (src) {
        result = cabx_i_str_dup_0(src, strlen(src) + 1);
    }
    return result;
}


/**
 * allocate memory
 */
void*
cabx_i_mem_alloc(
    size_t size)
{
    return malloc(size);
}

/**
 * free memory
 */
void
cabx_i_mem_free(
    void* heap_obj)
{
    free(heap_obj);
}



/* vi: se ts=4 sw=4 et: */
