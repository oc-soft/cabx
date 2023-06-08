#include "number_parser.h"
#include <stdlib.h>

/**
 * string to integer
 */
int
number_parser_str_to_int(
    const char* str,
    int base,
    int* value)
{
    int result;
    char* end_ptr;
    long l_value;
    end_ptr = NULL;
    result = 0;
    l_value = strtol(str, &end_ptr, base); 

    result = str != end_ptr ? 0 : -1;
    if (result == 0) {
        *value = (int)l_value;
    }
    return result;
}

/**
 * string to long with byte size modifier 
 */
int
number_parser_str_to_long_with_mod(
    const char* str,
    int base,
    unsigned long* value)
{
    int result;
    char* end_ptr;
    long l_value;
    result = 0;
    end_ptr = NULL;
    l_value = strtol(str, &end_ptr, base);

    result = end_ptr != str ? 0 : -1;

    if (result == 0) {
        char mod;
        mod = *end_ptr;

        if (mod == 'k' || mod == 'K') {
            l_value *= 0x400;
        } else if (mod == 'm' || mod == 'M') {
            l_value *= 0x100000;
        }
        *value = l_value;
    }
    return result;
}


/* vi: se ts=4 sw=4 et: */
