#include "cabx.h"
#include "cabx_i.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include "exe_info.h"

/**
 * cabinet generator
 */
struct _CABX {

    /**
     * reference count
     */
    unsigned int ref_count;


    /**
     * run main application
     */
    int (*run)(CABX*);
};

/**
 * show help message
 */
static int
cabx_show_help(
    CABX* obj);

/**
 * create cabinet generator instance
 */
CABX*
cabx_create()
{
    CABX* result;
    result = (CABX*)cabx_i_mem_alloc(sizeof(CABX));
    if (result) {
        result->ref_count = 1;
        result->run = cabx_show_help;
    }
    return result;
}


/**
 * increment reference count
 */
unsigned int
cabx_retain(
    CABX* obj)
{
    unsigned int result;
    result = 0;
    if (obj) {
        result = ++obj->ref_count;
    } else {
        errno = EINVAL;
    }
    return result;
}


/**
 * decrement reference count
 */
unsigned int
cabx_release(
    CABX* obj)
{
    unsigned int result;
    result = 0;
    if (obj) {
        result = --obj->ref_count;
        if (result == 0) {
            cabx_i_mem_free(obj);
        }
    } else {
        errno = EINVAL;
    }
    return result;
}

/**
 * parse command option 
 */
int
cabx_parse_option(
    CABX* obj,
    int argc,
    char* const * argv)
{
    int result;
    struct option options[] = {
        {
            .name = "help",
            .has_arg = no_argument,
            .flag = NULL,
            .val = 'h'
        },
        { NULL, 0, NULL, 0 }
    };
    result = 0;
    while (1) {
        int opt;
        opt = getopt_long(argc, argv, "h", options, NULL);

        switch (opt) {
            case 'h':
                obj->run = cabx_show_help;
                break;
        }

        if (opt == -1) {
            break;
        }
    }

    return result;    
}

/**
 * run cabinet generator
 */
int
cabx_run(
    CABX* obj)
{
    return obj->run(obj); 
}

/**
 * show help message
 */
static int
cabx_show_help()
{
    int result;
    char* exe_name;
    exe_name = exe_info_get_exe_name();
    result = 0;
    printf(
"%s [OPTIONS]\n"
"-h     show this message\n",
        exe_name);


    if (exe_name) {
        exe_info_free(exe_name);
    }

    return result;
}


/* vi: se ts=4 sw=4 et: */
