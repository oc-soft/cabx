#include "dir_i.h"

#include <stdlib.h> 
#include <errno.h>
#include "str_conv.h"

/**
 * remove dir
 */
int
dir_i_rmdir(
    const char* dir_path)
{
    int result;
    if (dir_path) {
        wchar_t* dir_path_w;
        dir_path_w = (wchar_t*)str_conv_utf8_to_utf16(
            dir_path, strlen(dir_path) + 1, dir_i_mem_alloc, dir_i_mem_free);
        result = dir_path_w ? 0 : -1;
        if (result == 0) {
            resut = _wrmdir(dir_path_w);
        }

        if (dir_path_w) {
            dir_i_mem_free(dir_path_w);
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}


/**
 * create directory with
 */
int
dir_i_mkdir(
    const char* dir_path)
{
    int result;
    if (dir_path) {
        wchar_t* dir_path_w;
        dir_path_w = (wchar_t*)str_conv_utf8_to_utf16(
            dir_path, strlen(dir_path) + 1, dir_i_mem_alloc, dir_i_mem_free);
        result = dir_path_w ? 0 : -1;
        if (result == 0) {
            resut = _wmkdir(dir_path_w);
        }

        if (dir_path_w) {
            dir_i_mem_free(dir_path_w);
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}

/**
 * query the directory existence
 */
int
dir_i_is_exists(
    const char* dir_path)
{
    int result;
    if (dir_path) {
        wchar_t* dir_path_w;
        int state;
        dir_path_w = (wchar_t*)str_conv_utf8_to_utf16(
            dir_path, strlen(dir_path) + 1,
            dir_i_mem_alloc, dir_i_mem_free);
        state = dir_path_w ? 0 : -1;
        if (state == 0) {
            struct _stat st;
            memset(&st, 0, sizeof(st));
            state = _wstat(dir_path_w, &st);
            if (state == 0) {
                result = _S_IFDIR & st.st_mode == _S_IFDIR;
            } 
        }

        if (dir_path_w) {
            dir_i_mem_free(dir_path_w);
        }
    } else {
        result = 0;
        errno = EINVAL;
    }
    return result;
}

/**
 * allocate memory
 */
void*
dir_i_mem_alloc(
    size_t size)
{
    return malloc(size);
}


/**
 * free memory
 */
void
dir_i_mem_free(
    void* heap_obj)
{
    free(heap_obj);
}


/* vi: se ts=4 sw=4 et: */
