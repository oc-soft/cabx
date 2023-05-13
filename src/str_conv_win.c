#include "str_conv.h"
#include <stdlib.h>
#include <Windows.h>

/**
 * allocate heap memory
 */
void*
str_conv_alloc(
    size_t size);


/**
 * convert utf8 to utf16 string
 */
void*
str_conv_utf8_to_utf16(
    const char* utf8_str,
    size_t size,
    void* (*alloc_mem)(size_t),
    void (*free_mem)(void*))
{
    int size_utf16;
    void* result;
    result = NULL;
    size_utf16 = MultiByteToWideChar(
        CP_UTF8, 0, utf8_str, size, NULL, 0);

    if (size_utf16) {
        void* buffer;
        void* (*alloc_mem_0)(size_t);
        void (*free_mem_0)(void*);
        if (alloc_mem) {
            alloc_mem_0 = alloc_mem;
        } else {
            alloc_mem_0 = str_conv_alloc;
        }
        if (free_mem) {
            free_mem_0 = free_mem;
        } else {
            free_mem_0 = str_conv_free;
        }
        buffer = alloc_mem_0(size_utf16 * sizeof(WCHAR));
        if (buffer) {
            int state;
            state = MultiByteToWideChar(
                CP_UTF8, 0, utf8_str, size, (LPWSTR)buffer, size_utf16);
            if (state) {
                result = buffer;
                buffer = NULL;
            }
        }
        
        if (buffer) {
            free_mem_0(buffer);
        } 
    }
    return result;
}

/**
 * convert utf16 to utf8 string
 */
char*
str_conv_utf16_to_utf8(
    const void* utf16_str,
    size_t size,
    void* (*alloc_mem)(size_t),
    void (*free_mem)(void*))
{
    int size_utf8;
    char* result;
    result = NULL;
    size_utf8 = WideCharToMultiByte(
        CP_UTF8, 0, (LPCWCH)utf16_str, size, NULL, 0, NULL, NULL);

    if (size_utf8) {
        char* buffer;
        void* (*alloc_mem_0)(size_t);
        void (*free_mem_0)(void*);
        if (alloc_mem) {
            alloc_mem_0 = alloc_mem;
        } else {
            alloc_mem_0 = str_conv_alloc;
        }
        if (free_mem) {
            free_mem_0 = free_mem;
        } else {
            free_mem_0 = str_conv_free;
        }
        buffer = (char*)alloc_mem_0(size_utf8);
        if (buffer) {
            int state;
            state = WideCharToMultiByte(
                CP_UTF8, 0, utf16_str, size, (LPSTR)buffer, size_utf8,
                NULL, NULL);
            if (state) {
                result = buffer;
                buffer = NULL;
            }
        }
        
        if (buffer) {
            free_mem_0(buffer);
        } 
    }
    return result;
}

/**
 * allocate heap memory
 */
void*
str_conv_alloc(
    size_t size)
{
    return malloc(size);
}


/**
 * free heap object
 */
void
str_conv_free(
    void* heap_obj)
{
    free(heap_obj);
}

/* vi: se ts=4 sw=4 et: */
