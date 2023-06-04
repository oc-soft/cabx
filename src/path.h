#ifndef __PATH_H__
#define __PATH_H__

#include <stddef.h>

#ifdef __cplusplus
#define _PATH_ITFC_BEGIN extern "C" {
#define _PATH_ITFC_END }
#else
#define _PATH_ITFC_BEGIN 
#define _PATH_ITFC_END 
#endif

_PATH_ITFC_BEGIN 


/**
 * remove begining directory separators
 */
int
path_remove_begin_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));

/**
 * remove end of directory separators
 */
int
path_remove_end_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));

/**
 * append directory separator
 */
int
path_append_dir_separator(
    const char* src,
    char** dst,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));

/**
 * get file spec
 */
int
path_get_file_spec(
    const char* path,
    char** file_spec,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));

/**
 * remove file spec
 */
int
path_remove_file_spec(
    const char* src_path,
    char** dst_path,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));


/**
 * concatenate path
 */
int
path_join(
    const char* path_1,
    const char* path_2,
    char** combined,
    void* (*mem_alloc)(size_t),
    void (*mem_free)(void*));

_PATH_ITFC_END 

/* vi: se ts=4 sw=4 et: */
#endif
