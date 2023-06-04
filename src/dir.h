#ifndef __DIR_H__
#define __DIR_H__

#include <stddef.h>

#ifdef __cplusplus
#define _DIR_ITFC_BEGIN extern "C" {
#define _DIR_ITFC_END }
#else
#define _DIR_ITFC_BEGIN 
#define _DIR_ITFC_END 
#endif

_DIR_ITFC_BEGIN 

/**
 * create directory with parent directory
 */
int
dir_mkdir_p(
    const char* dir_path);

/**
 * create directory
 */
int
dir_mkdir(
    const char* dir_path);

/**
 * remove directory
 */
int
dir_rmdir(
    const char* dir_path);


_DIR_ITFC_END
/* vi: se ts=4 sw=4 et: */
#endif
