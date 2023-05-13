#ifndef __CABX_H__
#define __CABX_H__

#ifdef __cplusplus
#define _CABX_ITFC_BEGIN extern "C" {
#define _CABX_ITFC_END }
#else
#define _CABX_ITFC_BEGIN 
#define _CABX_ITFC_END 
#endif

_CABX_ITFC_BEGIN 

/**
 * cabinet generator
 */
typedef struct _CABX CABX;


/**
 * create cabinet generator instance
 */
CABX*
cabx_create();


/**
 * increment reference count
 */
unsigned int
cabx_retain(
    CABX* obj);


/**
 * decrement reference count
 */
unsigned int
cabx_release(
    CABX* obj);

/**
 * parse command option 
 */
int
cabx_parse_option(
    CABX* obj,
    int argc,
    char* const * argv);

/**
 * run cabinet generator
 */
int
cabx_run(
    CABX* obj);

_CABX_ITFC_END 
/* vi: se ts=4 sw=4 et: */
#endif
