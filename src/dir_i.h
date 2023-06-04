#ifndef __DIR_I_H__
#define __DIR_I_H__


#ifdef __cplusplus
#define _DIR_I_ITFC_BEGIN extern "C" {
#define _DIR_I_ITFC_END }
#else
#define _DIR_I_ITFC_BEGIN 
#define _DIR_I_ITFC_END 
#endif

_DIR_I_ITFC_BEGIN 


/**
 * remove dir
 */
int
dir_i_rmdir(
    const char* dir_path);

/**
 * create directory with
 */
int
dir_i_mkdir(
    const char* dir_path);

/**
 * query the directory existence
 */
int
dir_i_is_exists(
    const char* dir_path);

/**
 * allocate memory
 */
void*
dir_i_mem_alloc(
    size_t size);

/**
 * free memory
 */
void
dir_i_mem_free(
    void* heap_obj);


_DIR_I_ITFC_END
/* vi: se ts=4 sw=4 et: */
#endif
