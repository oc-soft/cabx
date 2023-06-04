#ifndef __PATH_I_H__
#define __PATH_I_H__

#include <stddef.h>

#ifdef __cplusplus
#define _PATH_I_ITFC_BEGIN extern "C" {
#define _PATH_I_ITFC_END }
#else
#define _PATH_I_ITFC_BEGIN 
#define _PATH_I_ITFC_END 
#endif

_PATH_I_ITFC_BEGIN 


/**
 * get directory separators
 */
int
path_i_get_dir_separators(
    const char** separator,
    size_t* size);

/**
 * skip root path
 */
int
path_i_skip_root(
    const char* path,
    const char** subpath);


_PATH_I_ITFC_END 

/* vi: se ts=4 sw=4 et: */
#endif
