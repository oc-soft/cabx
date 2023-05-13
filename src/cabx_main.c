#include <windows.h>
#include <fci.h>
#include <unistd.h>
#include <iconv.h>
#include "cabx.h"
#include "str_conv.h"


/**
 * convert utf16 argument to utf8
 */
static char**
argv_to_utf8(
    int argc,
    wchar_t** argv);
/**
 * free argv
 */
void
free_utf8_argv(
    int argc,
    char** argv);

/**
 * allocate memory
 */
static void*
mem_alloc(
    size_t size);

/**
 * allocate memory
 */
static void
mem_free(
    void* heap_obj);



/**
 * entry point
 */
int
wmain(
    int argc,
    wchar_t** argv)
{
    int result;
    char** argv_utf8;
    CABX* cab;
    result = 0;
    cab = NULL;
    argv_utf8 = argv_to_utf8(argc, argv);

    result = argv_utf8 ? 0 : -1;

    if (result == 0) {
        cab = cabx_create();
        result = cab ? 0 : -1;
    }
    if (result == 0) {
        result = cabx_parse_option(cab, argc, argv_utf8);
    }
    if (result == 0) {
        cabx_run(cab);
    }

    if (cab) {
        cabx_release(cab);
    }

    if (argv_utf8) {
        free_utf8_argv(argc, argv_utf8);
    }

    return result;
}


/**
 * convert utf16 argument to utf8
 */
static char**
argv_to_utf8(
    int argc,
    wchar_t** argv)
{
    char** result;
    result = (char**)mem_alloc(argc * sizeof(char*));

    if (result) {
        int state;
        int idx[] = { 0, 0 };
        state = 0;
        for (idx[0] = 0; idx[0] < argc; idx[0]++) {
            char* str_utf8;
            str_utf8 = str_conv_utf16_to_utf8(
                argv[idx[0]], wcslen(argv[idx[0]]) + 1, mem_alloc, mem_free);
            if (!str_utf8) {
                state = -1;
                break;
            }
            result[idx[0]] = str_utf8;
        }
        if (state) {
            for (idx[1] = 0; idx[1] < idx[0]; idx[1]++) {
                mem_free(result[idx[1]]); 
            }
            mem_free(result);
            result = NULL;
        }
    }

    return result;
}


/**
 * free argv
 */
void
free_utf8_argv(
    int argc,
    char** argv)
{
    int idx;
    for (idx = 0; idx < argc; idx++) {
        mem_free(argv[idx]);
    }
    mem_free(argv);
}

/**
 * allocate memory
 */
static void*
mem_alloc(
    size_t size)
{
    return malloc(size);
}

/**
 * allocate memory
 */
static void
mem_free(
    void* heap_obj)
{
    free(heap_obj);
}


/* vi: se ts=4 sw=4 et: */
