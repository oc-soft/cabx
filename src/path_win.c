#include "path.h"
#include <pathcch.h>
#include <string.h>
#include "str_conv.h"


/**
 * remove file spec
 */
int
path_remove_file_spec(
    const char* src_path,
    char** dst_path,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;

    result = 0;

    if (src_path) {
        wchar_t* path_w;
        path_w = str_conv_utf8_to_utf16(src_path, strlen(src_path) + 1,
            mem_alloc, mem_free);
        result = path_w ? 0 : -1;
        if (result == 0) {
            char* removed_path;
            PathCchRemoveFileSpec(path_w, wcslen(path_w));

            removed_path = str_conv_utf16_to_utf8(
                path_w, wcslen(path_w) + 1, mem_alloc, mem_free);
            result = removed_path ? 0 : -1;
            if (result == 0) {
                *dst_path = removed_path;
            }
        }
        if (path_w) {
            mem_free(path_w);
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result; 
}
/* vi: se ts=4 sw=4 et: */
