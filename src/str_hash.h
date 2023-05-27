#ifndef __STR_HASH_H__
#define __STR_HAHS_H__
#include <stddef.h>

#ifdef __cplusplus
#define _STR_HASH_BEGIN extern "c" {
#define _STR_HASH_END }
#else
#define _STR_HASH_BEGIN 
#define _STR_HASH_END 
#endif

_STR_HASH_BEGIN 

/**
 * calculate hash code
 */
int
str_hash_0(
    const char* str,
    size_t str_size);

/**
 * calculate hash code
 */
int
str_hash_1(
    const char* str);


_STR_HASH_END
/* vi: se ts=4 sw=4 et: */
#endif
