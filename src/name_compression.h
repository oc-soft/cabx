#ifndef __NAME_COMPRESSION_H__
#define __NAME_COMPRESSION_H__

#ifdef __cplusplus

#define _NAME_COMPRESSION_ITFC_BEGIN extern "C" {
#define _NAME_COMPRESSION_ITFC_END }

#else

#define _NAME_COMPRESSION_ITFC_BEGIN 
#define _NAME_COMPRESSION_ITFC_END 

#endif

_NAME_COMPRESSION_ITFC_BEGIN 

/**
 * string to compression code
 */
int
name_compression_str_to_code(
    const char* str,
    int* code);

_NAME_COMPRESSION_ITFC_END 

/* vi: se ts=4 sw=4 et: */
#endif
