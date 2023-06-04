#include "path.h"
#include "path_i.h"
#include <errno.h>
#include <string.h>

/**
 * you get non zero if src is directory separator.
 */
static int
path_is_dir_seps(
    int path_element,
    const char* separators,
    size_t separators_size);

/**
 * remove begining diectory separators
 */
int
path_remove_begin_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;
    result = 0;
    if (src != NULL) {
        size_t len_src;
        char* path;
        len_src = strlen(src);
        if (len_src) {
            size_t dir_seps_size;
            const char* dir_seps;
            size_t idx;
            path_i_get_dir_separators(&dir_seps, &dir_seps_size);
            for (idx = 0; idx < len_src; idx++) {
                if (!path_is_dir_seps(
                    src[idx], dir_seps, dir_seps_size)) {
                    break;
                }
            }
            if (idx < len_src) {
                len_src -= idx;
            } else {
                /* src is composed by separators.*/
                len_src = 1;
                idx = len_src - 1;
            }
            path = (char*)mem_alloc(len_src + 1);
            result = path ? 0 : -1;
            if (result == 0) {
                memcpy(path, &src[idx], len_src);
                path[len_src] = '\0';
                *dst = path;
            }
        } else {
            path = (char*)mem_alloc(1);
            result = path ? 0 : -1;
            if (result == 0) {
                path[0] = '\0';
                *dst = path;
            }
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}

/**
 * remove end of diectory separators
 */
int
path_remove_end_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;
    result = 0;
    if (src) {
        size_t len_src;
        size_t idx;
        char* path;
        len_src = strlen(src);
        if (len_src) {
            size_t dir_seps_size;
            const char* dir_seps;
            path_i_get_dir_separators(&dir_seps, &dir_seps_size);
            for (idx = 0; idx < len_src; idx++) {
                if (!path_is_dir_seps(
                    src[len_src - 1 - idx], dir_seps, dir_seps_size)) {
                    break;
                }
            }
            if (idx < len_src) {
                len_src = len_src - idx;
            } else {
                len_src = 1;
            }
            path = (char*)mem_alloc(len_src + 1);
            result = path ? 0 : -1;
            if (result == 0) {
                memcpy(path, src, len_src);
                path[len_src] = '\0';
                *dst = path;
            }
        } else {
            path = (char*)mem_alloc(len_src + 1);
            result = path ? 0 : -1;
            if (result == 0) {
                path[0] = '\0';
                *dst = path;
            }
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}

/**
 * append directory separator
 */
int
path_append_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;
    result = 0;
    if (src) {
        size_t len;
        size_t dir_seps_size;
        const char* dir_seps;
        path_i_get_dir_separators(&dir_seps, &dir_seps_size);
        len = strlen(src);
        if (len) {
            char* path;
            if (!path_is_dir_seps(src[len - 1], dir_seps, dir_seps_size)) {
                path = (char*)mem_alloc(len + 2);
                result = path ? 0 : -1;
                if (result == 0) {
                    memcpy(path, src, len);
                    path[len] = dir_seps[0];
                    path[len + 1] = '\0';
                }
            } else {
                path = (char*)mem_alloc(len + 1);
                result = path ? 0 : -1;
                if (result == 0) {
                    memcpy(path, src, len + 1);
                }
            }
            if (result == 0) {
                *dst = path;
            }

        } else {
            char* path;
            path = (char*)mem_alloc(2);
            result = path ? 0 : -1;
            if (result == 0) {
                path[0] = dir_seps[0];
                path[1] = '\0';
                *dst = path;
            }
        }
    } else {
        result = -1;
        errno = EINVAL;
    }

    return result;
}


/**
 * get file spec
 */
int
path_get_file_spec(
    const char* path,
    char** file_spec,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;
    char* parent_path;
    parent_path = NULL;
    result = path_remove_file_spec(path, &parent_path, mem_alloc, mem_free);
    if (result == 0) {
        if (parent_path) {
            size_t dir_seps_size;
            const char* dir_seps;
            const char* ptr;
            size_t parent_path_length;
            size_t fspec_len;
            char* tmp_file_spec;

            path_i_get_dir_separators(&dir_seps, &dir_seps_size);
  
            parent_path_length = strlen(parent_path);
            ptr = path + parent_path_length;
            while (*ptr) {
                if (!path_is_dir_seps(*ptr, dir_seps, dir_seps_size)) {
                    break;
                }
                ptr++;
            }
            fspec_len = strlen(ptr);
            tmp_file_spec = (char*)mem_alloc(fspec_len + 1);
            result = tmp_file_spec ? 0 : -1;
            if (result == 0) {
                memcpy(tmp_file_spec, ptr, fspec_len + 1);
                *file_spec = tmp_file_spec;
            }

        } else {
            *file_spec = NULL;
        }
    }
    mem_free(parent_path);
    return result;
}


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
        size_t idx;
        size_t length;
        size_t dir_seps_size;
        char* removed_path;
        const char* dir_seps;
        const char* sub_path;
        dir_seps_size = 0;
        removed_path = NULL;
        sub_path = src_path;
        result = path_i_skip_root(src_path, &sub_path);
        if (result == 0) {
            size_t root_path_len;
            root_path_len = sub_path - src_path;
 
            path_i_get_dir_separators(&dir_seps, &dir_seps_size);
            length = strlen(sub_path);
            if (length) {
                int sep_mode;
                sep_mode = 0;
                for (idx = 0; idx < length; idx++) {
                    if (!sep_mode) {
                        sep_mode = path_is_dir_seps(
                            sub_path[length - 1 - idx],
                            dir_seps, dir_seps_size);
                    } else {
                        sep_mode = path_is_dir_seps(
                            sub_path[length - 1 - idx],
                            dir_seps, dir_seps_size);
                        if (sep_mode == 0) {
                            break;
                        }
                    }
                }
                removed_path = (char*)mem_alloc(
                    root_path_len + length - idx + 1); 
                result = removed_path ? 0 : -1;
                if (result == 0) {
                    memcpy(removed_path, src_path, root_path_len);
                    memcpy(removed_path + root_path_len,
                        sub_path, length - idx);
                    removed_path[root_path_len + length - idx] = '\0';
                }
            } else {
                removed_path = (char*)mem_alloc(root_path_len + 1);
                result = removed_path ? 0 : -1;
                if (result == 0) {
                    memcpy(removed_path, src_path, root_path_len);
                    removed_path[root_path_len] = '\0';
                }
            }
        }
        if (result == 0) {
            *dst_path = removed_path;
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result; 
}

/**
 * concatenate path
 */
int
path_join(
    const char* path_1,
    const char* path_2,
    char** combined,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*))
{
    int result;
    result = 0;
    if (path_1 != NULL && path_2 != NULL) {
        size_t len_1;
        size_t len_2;
        size_t copy_offset;
        char* normalized_path_1;
        char* normalized_path_2;
        char* path;
        size_t dir_seps_size;
        const char* dir_seps;
        normalized_path_1 = NULL;
        normalized_path_2 = NULL;
        copy_offset = 0;
        path = NULL;
        path_i_get_dir_separators(&dir_seps, &dir_seps_size);
        result = path_remove_end_dir_separator(
            path_1, &normalized_path_1,
            mem_alloc, mem_free);
        if (result == 0) {
            result = path_remove_begin_dir_separator(
                path_2, &normalized_path_2,
                mem_alloc, mem_free);
        }
        if (result == 0) {
            len_1 = strlen(normalized_path_1);
            len_2 = strlen(normalized_path_2);
            path = (char*)mem_alloc(len_1 + len_2 + 1 + 1);
            result = path ? 0 : -1;
        }
        if (result == 0) {
            memcpy(path, normalized_path_1, len_1);
            copy_offset += len_1;
        }
        if (result == 0 && len_1 && len_2) {
            int path_1_dir_sep;
            int path_2_dir_sep;
            path_1_dir_sep = path_is_dir_seps(normalized_path_1[len_1 - 1],
                dir_seps, dir_seps_size);
            path_2_dir_sep = path_is_dir_seps(normalized_path_2[0],
                dir_seps, dir_seps_size);
            if (!path_1_dir_sep && !path_2_dir_sep) {
                path[len_1] = dir_seps[0];
                copy_offset++;
            }
        }
        if (result == 0) {
            memcpy(path + copy_offset, normalized_path_2, len_2);
            copy_offset += len_2;
        }
        if (result == 0) {
            path[copy_offset] = '\0';
            *combined = path;
            path = NULL;
        }
        mem_free(normalized_path_1);
        mem_free(normalized_path_2);
        if (path) {
            mem_free(path);
        }
    } else {
        result = -1;
        errno = EINVAL;
    }

    return result;
}


/**
 * you get non zero if src is directory separator.
 */
static int
path_is_dir_seps(
    int path_element,
    const char* separators,
    size_t separators_size)
{
    int result;
    size_t idx;
    result = 0;
    for (idx = 0; idx < separators_size; idx++) {
        result = path_element == separators[idx];
        if (result) {
            break;
        }
    }
    return result;
}

/* vi: se ts=4 sw=4 et: */
