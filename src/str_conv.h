#ifndef __STR_CONV_H__
#define __STR_CONV_H__

#include <stddef.h>

#ifdef __cplusplus
#define _STR_CONV_ITFC_BEGIN extern "C" {
#define _STR_CONV_ITFC_END }
#else
#define _STR_CONV_ITFC_BEGIN 
#define _STR_CONV_ITFC_END 
#endif
_STR_CONV_ITFC_BEGIN 

/**
 * convert utf8 to utf16 string
 */
void*
str_conv_utf8_to_utf16(
    const char* utf8_str,
    size_t size,
    void* (*alloc_mem)(size_t),
    void (*free_mem)(void*));

/**
 * convert utf16 to utf8 string
 */
char*
str_conv_utf16_to_utf8(
    const void* utf16_str,
    size_t size,
    void* (*alloc_mem)(size_t),
    void (*free_mem)(void*));


/**
 * free heap object
 */
void
str_conv_free(
    void* heap_obj);


_STR_CONV_ITFC_END
/* vi: se ts=4 sw=4 et: */
#endif
