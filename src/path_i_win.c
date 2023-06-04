#include "path_i.h"
#include <stddef.h>
#include <winerror.h>
#include <pathcch.h>
#include <errno.h>
#include "str_conv.h"

/**
 * allocate memory
 */
static void*
path_i_mem_alloc(
    size_t size);

/**
 * free memory
 */
static void
path_i_mem_free(
    void* heap_obj);


/**
 * forwardslash to backslash
 */
static void
path_i_to_backslash(
    char* path);


/**
 * get directory separators
 */
int
path_i_get_dir_separators(
    const char** out_seps,
    size_t* size)
{
    int result;
    const static char separators[] = {
        '\\',
        '/'
    };
    result = 0;
    *out_seps = separators;
    *size = sizeof(separators);
    return result;
}


/**
 * skip root path
 */
int
path_i_skip_root(
    const char* path,
    const char** subpath)
{
    int result;
    if (path) {
        size_t path_len;
        path_len = strlen(path);
        if (path_len) {
            size_t idx;
            wchar_t* tmp_path_w;
            char* tmp_path[] = { NULL, NULL };
            tmp_path_w = NULL;
            tmp_path[0] = (char*)path_i_mem_alloc(path_len + 1);
            result = tmp_path[0] ? 0 : -1;
            if (result == 0) {
                memcpy(tmp_path[0], path, path_len + 1);
                path_i_to_backslash(tmp_path[0]);

                tmp_path_w = (wchar_t*)str_conv_utf8_to_utf16(
                    tmp_path[0], path_len + 1,
                    path_i_mem_alloc, path_i_mem_free);

                result = tmp_path_w ? 0 : -1;
            }
            if (result == 0) {
                const wchar_t* sub_path_start;
                size_t root_len_w;
                HRESULT h_state;
                sub_path_start = tmp_path_w;
                h_state = PathCchSkipRoot(
                    tmp_path_w, &sub_path_start);
                if (SUCCEEDED(h_state)) {
                    root_len_w = sub_path_start - tmp_path_w;
                    if (root_len_w) {
                        tmp_path_w[root_len_w] = L'\0';
                        tmp_path[1] = str_conv_utf16_to_utf8(
                            tmp_path_w, root_len_w + 1,
                            path_i_mem_alloc, path_i_mem_free);
                        result = tmp_path[1] ? 0 : -1;
                        if (result == 0) {
                            size_t root_len;
                            root_len = strlen(tmp_path[1]);
                            *subpath = path + root_len;
                        }
                    } else {
                        *subpath = path;
                    }
                } else {
                    *subpath = path;
                }
            }
            path_i_mem_free(tmp_path_w);
            for (idx = 0;
                idx < sizeof(tmp_path) / sizeof(tmp_path[0]);
                idx++) {
                path_i_mem_free(tmp_path[idx]);
            }

        } else {
            *subpath = path;
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}

/**
 * forwardslash to backslash
 */
static void
path_i_to_backslash(
    char* path)
{
    char* ptr;
    ptr = path;
    while (*ptr) {
        if (*ptr == '/') {
            *ptr = '\\';
        }
        ptr++;
    }
}


/**
 * allocate memory
 */
static void*
path_i_mem_alloc(
    size_t size)
{
    return malloc(size);
}

/**
 * free memory
 */
static void
path_i_mem_free(
    void* heap_obj)
{
    free(heap_obj);
}


/* vi: se ts=4 sw=4 et: */
