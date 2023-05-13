#include "exe_info.h"
#include <stdlib.h>
#include <Windows.h>
#include <pathcch.h>
#include "str_conv.h"

/**
 * allocate memory
 */
static void*
exe_info_alloc(
    size_t size);


/**
 * get module file path 
 */
static LPWSTR
get_module_path();

/**
 * get executable name
 */
char*
exe_info_get_exe_name()
{
    char* result;
    LPWSTR mod_path;
    result = NULL;
    mod_path = get_module_path();

    if (mod_path) {
        LPCWCHAR name_ptr;
        size_t i;
        int length;
        length = lstrlenW(mod_path);
        name_ptr = NULL;
        
        for (i = length - 1; i > 0; i--) {
            if (mod_path[i] == L'\\' || mod_path[i] == L'/') {
                name_ptr = &mod_path[i + 1];
                break;
            }
        }
        if (name_ptr) {
            result = (char*)str_conv_utf16_to_utf8(
                name_ptr, length - i, exe_info_alloc, exe_info_free);
        }
    }
    if (mod_path) {
        exe_info_free(mod_path);
    }
    return result;
}

/**
 * get executable directory
 */
char*
exe_info_get_exe_dir()
{
    char* result;
    LPWSTR mod_path;
    result = NULL;
    mod_path = get_module_path();

    if (mod_path) {
        size_t i;
        int length;
        length = lstrlenW(mod_path);
        PathCchRemoveFileSpec(mod_path, length + 1);
        result = (char*)str_conv_utf16_to_utf8(
            mod_path, lstrlenW(mod_path), exe_info_alloc, exe_info_free);
    }
    if (mod_path) {
        exe_info_free(mod_path);
    }
    return result;
}

/**
 * get module file path 
 */
static LPWSTR
get_module_path()
{
    LPWSTR result;
    size_t idx;
    result = NULL;
    for (idx = 0; idx < 3; idx++) {
        DWORD char_size;
        int do_continue;
        LPWSTR path_buffer;
        do_continue = 1;
        char_size = (MAX_PATH * (idx + 1) + 1);
        path_buffer = (LPWSTR)exe_info_alloc(
            sizeof(WCHAR) * char_size);

        if (path_buffer) {
            DWORD char_size_res;
            char_size_res = GetModuleFileNameW(NULL, path_buffer, char_size); 
            if (char_size_res > 0 && char_size >= char_size_res) {
                result = path_buffer;
                path_buffer = NULL;
                do_continue = 0;
            } else if (char_size_res == 0) {
                do_continue = 0;
            }
        }

        if (path_buffer) {
            exe_info_free(path_buffer);
        }
        if (!do_continue) {
            break;
        }
    }
    return result;
}



/**
 * free resource
 */
void
exe_info_free(
    void* obj)
{
    free(obj);
}

/**
 * allocate memory
 */
static void*
exe_info_alloc(
    size_t size)
{
    return malloc(size);
}


/* vi: se ts=4 sw=4 et: */

