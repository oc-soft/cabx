#include "str_hash.h"

#include <string.h>
#include <limits.h>

/**
 * calculate hash code
 */
int
str_hash_0(
    const char* str,
    size_t str_size)
{
    int result;
    size_t idx;
    size_t count[2];
    result = 0;
    count[0] = str_size / sizeof(int);
    count[1] = str_size % sizeof(int);
    for (idx = 0; idx < count[0]; idx++) {
        int tmp_value;
        size_t idx_0;
        tmp_value = 0;
        for (idx_0 = 0; idx_0 < sizeof(int); idx_0++) {
            int a_char;
            a_char = str[idx * sizeof(int) + idx_0];
            tmp_value |= a_char << (idx * CHAR_BIT);
        }
        result ^= tmp_value;
    }
    for (idx = 0; idx < count[1]; idx++) {
        char tmp_char;
        tmp_char = str[count[0] * sizeof(int) + idx];
        tmp_char = ~tmp_char;
        result |= tmp_char;
    }

    return result;
}

/**
 * calculate hash code
 */
int
str_hash_1(
    const char* str)
{
    int result;
    if (str) {
        result = str_hash_0(str, strlen(str));
    } else {
        result = 0;
    }
}
