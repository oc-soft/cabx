#ifndef __NUMBER_PARSER_H__
#define __NUMBER_PARSER_H__

#ifdef __cplusplus
#define _NUMBER_PARSER_ITFC_BEGIN extern "C" {
#define _NUMBER_PARSER_ITFC_END }
#else
#define _NUMBER_PARSER_ITFC_BEGIN 
#define _NUMBER_PARSER_ITFC_END 
#endif

_NUMBER_PARSER_ITFC_BEGIN 

/**
 * string to integer
 */
int
number_parser_str_to_int(
    const char* str,
    int base,
    int* value);

/**
 * string to long with byte size modifier 
 */
int
number_parser_str_to_long_with_mod(
    const char* str,
    int base,
    unsigned long* value);



_NUMBER_PARSER_ITFC_END 

/* vi: se ts=4 sw=4 et: */
#endif
