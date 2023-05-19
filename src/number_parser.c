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

/* vi: se ts=4 sw=4 et: */
