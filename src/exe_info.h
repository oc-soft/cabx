#ifndef __EXE_INFO_H__
#define __EXE_INFO_H__

#ifdef __cplusplus
#define _EXE_INFO_BEGIN extern "c" {
#define _EXE_INFO_END }
#else
#define _EXE_INFO_BEGIN 
#define _EXE_INFO_END 
#endif

_EXE_INFO_BEGIN 

/**
 * get executable name
 */
char*
exe_info_get_exe_name();

/**
 * get executable directory
 */
char*
exe_info_get_exe_dir();


/**
 * free resource
 */
void
exe_info_free(
    void* obj);
_EXE_INFO_END
/* vi: se ts=4 sw=4 et: */
#endif
