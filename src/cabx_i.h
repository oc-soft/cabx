#ifndef __CABX_I_H__
#define __CABX_I_H__

#include <stddef.h>

#ifdef __cplusplus
#define _CABX_I_ITFC_BEGIN extern "C" {
#define _CABX_I_ITFC_END }
#else
#define _CABX_I_ITFC_BEGIN 
#define _CABX_I_ITFC_END 
#endif

_CABX_I_ITFC_BEGIN 


/**
 * duplicate string
 */
char*
cabx_i_str_dup_0(
    const char* src,
    size_t length);

/**
 * duplicate string
 */
char*
cabx_i_str_dup(
    const char* src);


/**
 * allocate memory
 */
void*
cabx_i_mem_alloc(
    size_t size);

/**
 * free memory
 */
void
cabx_i_mem_free(
    void* heap_obj);


_CABX_I_ITFC_END 

/* vi: se ts=4 sw=4 et: */
#endif
