#include "cabx.h"
#include "cabx_i.h"
#include <windows.h>
#include <fci.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <limits.h>
#include <direct.h>
#include "exe_info.h"
#include "col/array_list.h"
#include "col/list_ref.h"
#include "col/rb_map.h"
#include "cstr.h"
#include "csv.h"
#include "buffer/char_buffer.h"
#include "buffer/variable_buffer.h"
#include "name_compression.h"
#include "number_parser.h"
#include "str_conv.h"
#include "str_hash.h"
#include "path.h"

/**
 * option for cabinet genertor
 */
typedef struct _CABX_OPTION CABX_OPTION;

/**
 * entry for a cabinet
 */
typedef struct _CABX_ENTRY CABX_ENTRY;

/**
 * entry iteration status
 */
typedef struct _CABX_ENTRY_ITER_STATE CABX_ENTRY_ITER_STATE;

/**
 * entry iteration status for report
 */
typedef struct _CABX_ENTRY_ITER_REPORT_STATE CABX_ENTRY_ITER_REPORT_STATE;


/**
 * cabinet generation status
 */
typedef struct _CABX_GENERATION_STATUS CABX_GENERATION_STATUS;

/**
 * cabinet generator
 */
struct _CABX {

    /**
     * reference count
     */
    unsigned int ref_count;


    /**
     * cab file entries
     */
    col_list* entries;

    /**
     * source path to entry name map
     */
    col_map* source_path_entry_map;


    /**
     * entry cabinet name map
     */
    col_map* entry_cab_map;

    /**
     * cabinet name to entries map
     */
    col_map* cab_entries_map;

    /**
     * cabinet name to output directory map
     */
    col_map* cab_outdir_map;

    /**
     * run main application
     */
    int (*run)(CABX*);

    /**
     * option
     */
    CABX_OPTION* option;
};

/**
 * option for cabinet genertor
 */
struct _CABX_OPTION {
    /**
     * show status if this flag is not zero
     */
    int show_status;

    /**
     * input file for entries
     * It uses standard input if input is -
     */
    char* input;

    /**
     * output directory into where cabinet file be created.
     */
    char* output_dir;

    /**
     * cabinet name
     */
    char* cabinet_name;

    /**
     * disk name
     */
    char* disk_name;

    /**
     * cabinet size
     */
    unsigned long max_cabinet_size;


    /**
     * folder size
     */
    unsigned long folder_threshold;


    /**
     * report file
     */
    char* report_file;
};

/**
 * cab entry
 */
struct _CABX_ENTRY {

    /**
     * reference count
     */
    unsigned int ref_count;

    /**
     * source file path
     */
    char* source_file;
    
    /**
     * cab entry name
     */
    char* entry_name;

    /**
     * compression kind
     */
    int compression;

    /**
     * data attribute
     */
    int attribute;

    /**
     * has executing flag
     */
    int execute;

    /**
     * flush folder flat
     */
    int flush_folder;

    /**
     * flush cabinet flag
     */
    int flush_cabinet;
};

/**
 * entry iteration status
 */
struct _CABX_ENTRY_ITER_STATE {
    /**
     * last compression type
     */

    int last_compression_type;

    /**
     * processed count
     */
    size_t processed_count;

    /**
     * fci handle
     */
    HFCI fci_handle;


    /**
     * generation status
     */
    CABX_GENERATION_STATUS* generation_status;
};

/**
 * entry iteration status for reporting
 */
struct _CABX_ENTRY_ITER_REPORT_STATE {

    /**
     * cabinet extension object
     */
    CABX* cabx;


    /**
     * output stream
     */
    FILE* output_stream;
};



/**
 * generation status
 */
struct _CABX_GENERATION_STATUS {

    /**
     * cabinet object
     */
    CABX* cabx;

    /**
     * cabinet configuration
     */
    CCAB* ccab;


    /**
     * end of generation
     */
    int end_of_generation;


    /**
     * last file entry 
     */
    cstr* last_file_entry;

    /**
     * the begining cabinet name when the file is continuation for cabinet.
     */
    cstr* starting_cabinet_name;

    /**
     * next cabinet name
     */
    cstr* next_cabinet_name;

    /**
     * next disk name
     */
    cstr* next_disk_name;
};


/**
 * show help message
 */
static int
cabx_show_help(
    CABX* obj);

/**
 * generate cabinet
 */
static int
cabx_generate(
    CABX* obj);
/**
 * add an entry
 */
static int
cabx_add_entry(
    CABX* obj,
    const char* source_path,
    const char* entry_name,
    int compression_code,
    int attribute,
    int execute,
    int flush_folder,
    int flush_cabinet);

/**
 * load entries 
 */
static int
cabx_load_entries(
    CABX* obj);

/**
 * create cabinet
 */
static int
cabx_create_cab(
    CABX* obj,
    HFCI fci_hdl,
    CABX_GENERATION_STATUS* generation_status);

/**
 * get non zero if all entries are belong into a cabinet.
 */
static int
cabx_is_generated_all(
    CABX* obj);


/**
 * report entry name cabinet map
 */
static int
cabx_report_cab_map(
    CABX* obj);

/**
 * put cabinet output directory
 */
static int
cabx_put_cabinet_output_dir(
    CABX* obj,
    const char* cabinet_name,
    const char* output_dir);

/**
 * remove last cabinet if it is empty
 */
static int
cabx_remove_last_cab_if_empty(
    CABX* obj,
    cstr* outdir,
    cstr* cabinet_name);

/**
 *  handle file path to for output directory
 */
static int
cabx_handle_file_path_for_output_dir(
    CABX* obj,
    const char* file_path,
    int (*run)(CABX*, const char*, const char*, const char*));

/**
 * create output directory if directry is not exist.
 */
static int
cabx_create_output_dir_if_not(
    CABX* obj,
    const char* file_path,
    const char* output_dir,
    const char* file_name);



/**
 * load entries from csv
 */
static int
cabx_load_entries_from_csv(
    CABX* obj,
    csv* csv_in);

/**
 * load csv input raw data 
 */
static int
cabx_load_csv_input(
    CABX* obj,
    char** src,
    size_t* src_size);

/**
 * load csv from input
 */
static csv*
cabx_load_csv_from_input(
    CABX* obj);

/**
 * fill fci cab parameter
 */
static int
cabx_fill_cab_param(
    CABX* obj,
    CCAB* param);

/**
 * fill cabinet name
 */
static int
cabx_fill_cabinet_name(
    CABX* obj,
    int cab_index,
    char* buffer,
    size_t buffer_size);

/**
 * fill disk name
 */
static int
cabx_fill_disk_name(
    CABX* obj,
    int disk_index,
    char* buffer,
    size_t buffer_size);

/**
 * copy cab path into buffer 
 */
static int
cabx_fill_cab_path(
    CABX* obj,
    const char* cab_name,
    char* buffer,
    size_t buffer_size);

/**
 * entry iterator
 */
static int
cabx_entries_iter(
    CABX_ENTRY_ITER_STATE* iter_state,
    CABX_ENTRY* entry);
/**
 * You get non zero if entry is last element.
 */
static int
cabx_entries_iter_is_end_of_entry(
    CABX_ENTRY_ITER_STATE* iter_state);

/**
 * entry iterator
 */
static int
cabx_entries_iter_for_reporting(
    CABX_ENTRY_ITER_REPORT_STATE* iter_state,
    CABX_ENTRY* entry);

/**
 * create option instance
 */
static CABX_OPTION*
cabx_option_create();

/**
 * free option instance
 */
static void
cabx_option_free(
    CABX_OPTION* opt);

/**
 * set input into option
 */
static int
cabx_option_set_input(
    CABX_OPTION* opt,
    const char* input);

/**
 * set output directory into option
 */
static int
cabx_option_set_output_dir(
    CABX_OPTION* opt,
    const char* output_dir);

/**
 * set cabinet name into option
 */
static int
cabx_option_set_cabinet_name(
    CABX_OPTION* opt,
    const char* cabinet_name);

/**
 * set disk name into option
 */
static int
cabx_option_set_disk_name(
    CABX_OPTION* opt,
    const char* disk_name);

/**
 * set max cabinet size by string format into option
 */
static int
cabx_option_set_max_cabinet_size(
    CABX_OPTION* opt,
    const char* max_size);

/**
 * set cabinet folder threshold size by string format into option
 */
static int
cabx_option_set_folder_threshold(
    CABX_OPTION* opt,
    const char* threshold_size);


/**
 * set report file into option
 */
static int
cabx_option_set_report(
    CABX_OPTION* opt,
    const char* output);



/**
 * cab entry
 */
static CABX_ENTRY*
cabx_entry_create_0();

/**
 * cab entry
 */
static CABX_ENTRY*
cabx_entry_create_1(
    const char* src_name,
    const char* entry_name,
    int compression_code,
    int attribute,
    int executing,
    int flush_folder,
    int flush_cabinet);


/**
 * increment reference count
 */
static unsigned int
cabx_entry_retain(
    CABX_ENTRY* entry);

/**
 * decrement reference count
 */
static unsigned int
cabx_entry_release(
    CABX_ENTRY* entry);

/**
 * decrement reference count
 */
static void
cabx_entry_release_1(
    CABX_ENTRY* entry);

/**
 * copy entry object
 */
static int
cabx_entry_copy_ref(
    CABX_ENTRY* src_entry,
    CABX_ENTRY** dst_entry);

/**
 * calculate hash code
 */
static int
cabx_entry_hash_code(
    CABX_ENTRY* entry);

/**
 * set source file
 */
int
cabx_entry_set_source_file(
    CABX_ENTRY* entry,
    const char* source_file);

/**
 * set entry name
 */
static int
cabx_entry_set_entry_name(
    CABX_ENTRY* entry,
    const char* entry_name);

/**
 * encode  string
 */
static int
cabx_encode_str(
    const char* src,
    char** encoded,
    int* attribute);

/**
 * decode string
 */
static int
cabx_decode_str(
    const char* src,
    char** decoded);

/**
 * set last file entry 
 */
static int
cabinet_generation_status_set_last_file_entry(
    CABX_GENERATION_STATUS* obj,
    cstr* last_file_entry);


/**
 * set starting cabinet name 
 */
static int
cabinet_generation_status_set_starting_cabinet_name(
    CABX_GENERATION_STATUS* obj,
    cstr* cabinet_name);

/**
 * set next cabinet name
 */
static int
cabinet_generation_status_set_next_cabinet_name(
    CABX_GENERATION_STATUS* obj,
    cstr* next_cabinet_name);


/**
 * set disk name 
 */
static int
cabinet_generation_status_set_next_disk_name(
    CABX_GENERATION_STATUS* obj,
    cstr* disk_name);


/**
 * add entry into cab name to entries map
 */
static int
cabx_cab_entries_add_entry(
    col_map* cab_entries_map,
    cstr* cab_name,
    cstr* entry_name);

/**
 * get entry count of cab name to entries map
 */
static int
cabx_cab_entries_get_entry_count(
    col_map* cab_entries_map,
    cstr* cab_name,
    size_t* count);

/**
 * allocate memory for fci
 */
static void* 
cabx_fci_alloc(unsigned long size);

/**
 * free memory for fci
 */
static void 
cabx_fci_free(void* heap_obj);

/**
 * open file for fci
 */
static intptr_t 
cabx_fci_open(
    LPSTR file_path,
    int open_flag,
    int mode,
    int *err,
    void* user_data);
/**
 * read data for fci
 */
static unsigned int
cabx_fci_read(
    intptr_t file_hdl,
    void* buffer,
    unsigned int buffer_size,
    int* err,
    void* user_data);

/**
 * write data for fci
 */
static unsigned int
cabx_fci_write(
    intptr_t file_hdl,
    void* buffer,
    unsigned int buffer_size,
    int* err,
    void* user_data);

/**
 * seek file for fci
 */
static long 
cabx_fci_seek(
    intptr_t file_hdl,
    long dist,
    int seek_type,
    int *err,
    void* user_data);

/**
 * close file for fci
 */
static int
cabx_fci_close(
    intptr_t file_hdl,
    int *err,
    void* user_data);
/**
 * delete file for fci
 */
static int
cabx_fci_delete(
    LPSTR file_path,
    int *err,
    void* user_data);
/**
 * be called when a file is placed in cabinet.
 */
static int
cabx_fci_file_placed(
    PCCAB pccab,
    char* file,
    long file_size,
    BOOL file_continuation,
    void* user_data);

/**
 * get temporary file name
 */
static int
cabx_fci_get_temporary_file_name(
    char* temporary_file_path,
    int file_path_size,
    void* user_data);

/**
 * control next cabinet generating operation
 */
static int
cabx_fci_get_next_cabinet(
    CCAB* cab_param,
    unsigned long previous_cab,
    void* user_data);


/**
 * progress
 */
static long 
cabx_fci_progress(
    unsigned int status,
    unsigned long data_1,
    unsigned long data_2,
    void* user_data);

/**
 * get file date time attribute as fat format
 */
static intptr_t
cabx_fci_get_open_info(
    LPSTR file_path,
    unsigned short* data,
    unsigned short* time,
    unsigned short* attr,
    int* err,
    void* user_data);


/**
 * default max cabinet size
 */
const unsigned long CABX_MAX_CABINET_SIZE_DEF = ULONG_MAX;

/**
 * default folder threshold size
 */
const unsigned long CABX_FOLDER_THRESHOLD_DEF = ULONG_MAX;


/**
 * create cabinet generator instance
 */
CABX*
cabx_create()
{
    CABX* result;
    CABX_OPTION* option;
    col_list* entries;
    col_map* source_path_entry_map;
    col_map* entry_cab_map;
    col_map* cab_entries_map;
    col_map* cab_outdir_map;
    result = (CABX*)cabx_i_mem_alloc(sizeof(CABX));
    option = cabx_option_create();
    entries = col_array_list_create(
        10, 10,
        (int (*)(void*))cabx_entry_hash_code,
        (int (*)(const void*, void**))cabx_entry_copy_ref,
        (void (*)(void*))cabx_entry_release_1);

    source_path_entry_map = col_rb_map_create(
        (int (*)(const void*, const void*))cstr_compare,
        (unsigned int (*)(const void*))cstr_hash,
        (unsigned int (*)(const void*))cstr_hash,
        (int (*)(const void*, void**))cstr_retain_1,
        (int (*)(const void*, void**))cstr_retain_1,
        (void (*)(void*))cstr_release_1,
        (void (*)(void*))cstr_release_1);

    entry_cab_map = col_rb_map_create(
        (int (*)(const void*, const void*))cstr_compare,
        (unsigned int (*)(const void*))cstr_hash,
        (unsigned int (*)(const void*))cstr_hash,
        (int (*)(const void*, void**))cstr_retain_1,
        (int (*)(const void*, void**))cstr_retain_1,
        (void (*)(void*))cstr_release_1,
        (void (*)(void*))cstr_release_1);

    cab_outdir_map = col_rb_map_create(
        (int (*)(const void*, const void*))cstr_compare,
        (unsigned int (*)(const void*))cstr_hash,
        (unsigned int (*)(const void*))cstr_hash,
        (int (*)(const void*, void**))cstr_retain_1,
        (int (*)(const void*, void**))cstr_retain_1,
        (void (*)(void*))cstr_release_1,
        (void (*)(void*))cstr_release_1);

    cab_entries_map = col_rb_map_create(
        (int (*)(const void*, const void*))cstr_compare,
        (unsigned int (*)(const void*))cstr_hash,
        (unsigned int (*)(const void*))col_list_ref_hash,
        (int (*)(const void*, void**))col_list_ref_copy_1,
        (int (*)(const void*, void**))cstr_retain_1,
        (void (*)(void*))col_list_ref_release,
        (void (*)(void*))cstr_release_1);

    if (result && option && entries && cab_outdir_map 
        && source_path_entry_map && entry_cab_map && cab_entries_map) {
        result->ref_count = 1;
        result->run = cabx_generate;
        result->option = option;
        result->entries = entries;
        result->source_path_entry_map = source_path_entry_map;
        result->entry_cab_map = entry_cab_map;
        result->cab_outdir_map = cab_outdir_map; 
        result->cab_entries_map = cab_entries_map;
    } else {
        if (cab_entries_map) {
            col_map_free(cab_entries_map);
        }
        if (cab_outdir_map) {
            col_map_free(cab_outdir_map); 
        }
        if (entry_cab_map) {
            col_map_free(entry_cab_map);
        }
        if (source_path_entry_map) {
            col_map_free(source_path_entry_map);
        }
        if (entries) {
            col_list_free(entries);
        }
        if (option) {
            cabx_option_free(option);
        }
        if (result) {
            cabx_i_mem_free(result);
            result = NULL;
        }
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
            col_map_free(obj->cab_entries_map);
            col_map_free(obj->entry_cab_map);
            col_map_free(obj->cab_outdir_map); 
            col_map_free(obj->source_path_entry_map);
            col_list_free(obj->entries);
            cabx_option_free(obj->option);
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
            .name = "input",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'i'
        },
        {
            .name = "output",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'o'
        },
        {
            .name = "cab-name",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'c'
        },
        {
            .name = "disk-name",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'd'
        },
        {
            .name = "max-cabinet",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'm'
        },
        {
            .name = "folder-thresh",
            .has_arg = required_argument,
            .flag = NULL,
            .val = 'f'
        },
        {
            .name = "report",
            .has_arg = optional_argument,
            .flag = NULL,
            .val = 'r'
        },
        {
            .name = "show-status",
            .has_arg = no_argument,
            .val = 's'
        },
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
        opt = getopt_long(argc, argv,
            "i:o:d:c:m:f:r::hs", options, NULL);

        switch (opt) {
            case 'i':
                result = cabx_option_set_input(obj->option, optarg);
                break;
            case 'c':
                result = cabx_option_set_cabinet_name(obj->option, optarg);
                break;
            case 'd':
                result = cabx_option_set_disk_name(obj->option, optarg);
                break;
            case 'm':
                result = cabx_option_set_max_cabinet_size(obj->option, optarg);
                break;
            case 'f':
                result = cabx_option_set_folder_threshold(obj->option, optarg);
                break;
            case 'r':
                result = cabx_option_set_report(obj->option, optarg);
                break;
            case 'o':
                result = cabx_option_set_output_dir(obj->option, optarg);
                break;
            case 's':
                obj->option->show_status = 1;
                break;
            case 'h':
                obj->run = cabx_show_help;
                break;
            default:
                break;
        }
        if (result) {
            break;
        }
        if (opt == -1) {
            break;
        }
    }

    return result;    
}


/**
 * show help message
 */
static int
cabx_show_help(
    CABX* obj)
{
    int result;
    char* exe_name;
    exe_name = exe_info_get_exe_name();
    result = 0;
    printf(
"%s [OPTIONS]\n"
"-i, --input= [INPUT]               specify cab entry csv file.\n"
"                                   default is - which means standard input.\n"
"-o, --output= [OUTPUT]             specify output directory.\n"
"                                   default current working directory.\n"
"-c, --cab-name= [CAB]              specify cabinet file name template.\n"
"                                   default \"data%%d.cab\"\n"
"-d, --disk-name= [DISK]            specify disk name template.\n"
"                                   default is empty (\"\") which means\n"
"                                   not using disk\n"
"-m, --max-cabinet= [SIZE][k|m]     specify maximum cabinet size.\n"
"                                   default is %lu bytes\n"
"-f, --folder-thresh= [SIZE][k|m]   specify folder threshold size.\n"
"                                   default is %lu bytes\n"
"-r, --report= [FILE]               specify report file.\n"
"                                   if you set \"-\" as file, then print to\n"
"                                   stdout.\n"
"-s, --show-status                  show proccessing status.\n"
"-h                                 show this message\n",
        exe_name,
        CABX_MAX_CABINET_SIZE_DEF,
        CABX_FOLDER_THRESHOLD_DEF);


    if (exe_name) {
        exe_info_free(exe_name);
    }

    return result;
}

/**
 * load entries 
 */
static int
cabx_load_entries(
    CABX* obj)
{
    int result;
    csv* csv_input;
    result = 0;

    csv_input = cabx_load_csv_from_input(obj);
    result = csv_input ? 0 : -1;
    if (result == 0) {
        result = cabx_load_entries_from_csv(obj, csv_input);
    }

    if (csv_input) {
        csv_release(csv_input);
    }
    return result;
}
 
/**
 * load entries from csv
 */
static int
cabx_load_entries_from_csv(
    CABX* obj,
    csv* csv_in)
{
    int result;
    unsigned int r_idx;
    result = 0;
    for (r_idx = 0; r_idx < csv_get_row_count(csv_in); r_idx++) {
        char* source_path;
        char* entry_name;
        char* compression_str;
        char* attr_str;
        char* execute_str;
        char* flush_folder_str;
        char* flush_cabinet_str;
        result = 0;
        source_path = NULL;
        entry_name = NULL;
        compression_str = NULL;
        attr_str = NULL;
        execute_str = NULL;
        flush_folder_str = NULL;
        flush_cabinet_str = NULL;
        csv_get_value(csv_in, r_idx, 0, &source_path);
        csv_get_value(csv_in, r_idx, 1, &entry_name);
        csv_get_value(csv_in, r_idx, 2, &compression_str);
        csv_get_value(csv_in, r_idx, 3, &attr_str);
        csv_get_value(csv_in, r_idx, 4, &execute_str);
        csv_get_value(csv_in, r_idx, 5, &flush_folder_str);
        csv_get_value(csv_in, r_idx, 6, &flush_cabinet_str);

        if (source_path && entry_name && compression_str && attr_str) {
            int compression_code;
            int state;
            int attr;
            int execute;
            int flush_folder;
            int flush_cabinet;
            cstr* source_path_cstr;
            cstr* entry_name_cstr;
            attr = 0;
            compression_code = 0;
            execute = 0;
            flush_folder = 0;
            flush_cabinet = 0;
            source_path_cstr = NULL;
            entry_name_cstr = NULL;
            state = number_parser_str_to_int(attr_str, 10, &attr);

            if (state == 0 && execute_str && strlen(execute_str)) {
                state = number_parser_str_to_int(
                    execute_str, 10, &execute);
            }
            if (state == 0 && flush_folder_str && strlen(flush_folder_str)) {
                state = number_parser_str_to_int(
                    flush_folder_str, 10, &flush_folder);
            }
            if (state == 0 && flush_cabinet_str && strlen(flush_cabinet_str)) {
                state = number_parser_str_to_int(
                    flush_cabinet_str, 10, &flush_cabinet);
            }

            if (state == 0) {
                state = name_compression_str_to_code(
                    compression_str, &compression_code);        
            }
            if (state == 0) {
                state = cabx_add_entry(
                    obj, source_path, entry_name, compression_code, attr,
                    execute, flush_folder, flush_cabinet);
            }

            if (state == 0) {
                source_path_cstr = cstr_create_00(
                    source_path, strlen(source_path),
                    (void* (*)(unsigned int))cabx_i_mem_alloc,
                    cabx_i_mem_free);
                state = source_path_cstr ? 0 : -1;
            }
            if (state == 0) {
                entry_name_cstr = cstr_create_00(
                    entry_name, strlen(entry_name),
                    (void* (*)(unsigned int))cabx_i_mem_alloc, 
                    cabx_i_mem_free);
            }
            if (state == 0) {
                state = col_map_put(obj->source_path_entry_map,
                    source_path_cstr, entry_name_cstr);
            }
            if (source_path_cstr) {
                cstr_release(source_path_cstr);
            }
            if (entry_name_cstr) {
                cstr_release(entry_name_cstr);
            }
            result = state;
        }
        {
            size_t res_idx;
            char* str_res[] = {
                source_path,
                entry_name,
                compression_str,
                attr_str,
                execute_str,
                flush_folder_str,
                flush_cabinet_str
            };
            for (res_idx = 0;
                res_idx < sizeof(str_res) / sizeof(str_res[0]); 
                res_idx++) {
                cabx_i_mem_free(str_res[res_idx]);
            }
        }  
        if (result) {
            break;
        }
    }
    return result;
}

/**
 * add an entry
 */
static int
cabx_add_entry(
    CABX* obj,
    const char* source_path,
    const char* entry_name,
    int compression_code,
    int attribute,
    int execute,
    int flush_folder,
    int flush_cabinet)
{
    int result;
    CABX_ENTRY* entry;
    result = 0;

    entry = cabx_entry_create_1(
        source_path, entry_name, compression_code, attribute,
        execute, flush_folder, flush_cabinet);
    result = entry ? 0 : -1; 

    if (result == 0) {
        result = col_list_append(obj->entries, entry);
    }

    if (entry) {
        cabx_entry_release(entry);
    }

    return result;
}

/**
 * load csv from input
 */
static csv*
cabx_load_csv_from_input(
    CABX* obj)
{
    char* input_data;
    size_t input_data_size;
    csv* result;
    int state;

    result = NULL;
    input_data = NULL;
    input_data_size = 0;

    state = cabx_load_csv_input(obj, &input_data, &input_data_size);

    if (state == 0) {
        result = csv_create_1(
            (void* (*)(unsigned int))cabx_i_mem_alloc, 
            cabx_i_mem_free);
        state = result ? 0 : -1;
    }

    if (state == 0) {
        const char* new_lines[] = {
            "\n",
            "\r\n"
        };
        state = csv_load(result, input_data, input_data_size,
            ",", new_lines, sizeof(new_lines) / sizeof(new_lines[0]));

        if (state) {
            csv_release(result);
            result = NULL;
        }
    }
    return result;
}

/**
 * load csv input raw data 
 */
static int
cabx_load_csv_input(
    CABX* obj,
    char** src,
    size_t* src_size)
{
    int result;
    FILE* fi;
    buffer_char_buffer* buffer;
    fi = NULL;
    buffer = NULL;
    if (strcmp(obj->option->input, "-") == 0) {
        fi = stdin;
    } else {
        fi = fopen(obj->option->input, "r");
    }
    result = fi ? 0 : -1;

    if (result == 0) {
        buffer = buffer_char_buffer_create(100);
        result = buffer ? 0 : -1;
    }

    if (result == 0) {
        char* f_buffer;
        const static size_t f_buffer_size = 1024;
        f_buffer = (char*)cabx_i_mem_alloc(f_buffer_size);
        result = f_buffer ? 0 : -1;
        while (result == 0) {
            size_t read_size;
            read_size = 0;
            read_size = fread(f_buffer, 1, f_buffer_size, fi);

            if (read_size) {
                result = buffer_char_buffer_append(
                    buffer, f_buffer, read_size);
            } else {
                break;
            }
        }

        if (f_buffer) {
            cabx_i_mem_free(f_buffer);
        }
    }

    if (result == 0) {
        char* tmp_src;
        tmp_src = (char*)cabx_i_mem_alloc(
            buffer_char_buffer_get_size(buffer));
        result = tmp_src ? 0 : -1;

        if (result == 0) {
            buffer_char_buffer_copy_to(buffer, tmp_src);
            *src = tmp_src;
            *src_size = buffer_char_buffer_get_size(buffer);
            tmp_src = NULL;
        }

        if (tmp_src) {
            cabx_i_mem_free(tmp_src);
        }
    }
    if (buffer) {
        buffer_char_buffer_release(buffer);
    }

    if (fi) {
        if (strcmp(obj->option->input, "-") == 0) {
            fclose(fi);
        }
    }
    return result;
}

/**
 * entry iterator
 */
static int
cabx_entries_iter(
    CABX_ENTRY_ITER_STATE* iter_state,
    CABX_ENTRY* entry)
{
    int result;
    char* encoded_name;
    int encoded_attr;
    wchar_t* entry_name_w;
    size_t count_of_entries;

    result = 0;
    encoded_attr = 0;
    encoded_name = NULL;
    entry_name_w = str_conv_utf8_to_utf16(
        entry->entry_name, strlen(entry->entry_name) + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);

    count_of_entries = col_list_size(
        iter_state->generation_status->cabx->entries);
    if (iter_state->last_compression_type == tcompBAD) {
        iter_state->last_compression_type = entry->compression;
    } else {
        int flush_cabinet;
        flush_cabinet = iter_state->last_compression_type != entry->compression;
        if (flush_cabinet) {
            int state;
            state = FCIFlushCabinet(iter_state->fci_handle,
                TRUE,
                cabx_fci_get_next_cabinet,
                cabx_fci_progress);
            result = state ? 0 : -1;
        }
    }

    result = cabx_encode_str(entry->entry_name, &encoded_name, &encoded_attr);
    
    if (result == 0) {
        int state;
        state = FCIAddFile(iter_state->fci_handle,
            entry->source_file,
            encoded_name,
            entry->execute ? TRUE : FALSE,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress,
            cabx_fci_get_open_info,
            entry->compression);
        
        result = state ? 0 : -1;
    }
    if (result == 0) {
        iter_state->last_compression_type = entry->compression;
        iter_state->processed_count++;
    }

    if (result == 0 && entry->flush_folder) {
        int state;
        state = FCIFlushFolder(iter_state->fci_handle,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        result = state ? 0 : -1;
    }
  
    if (result == 0 && entry->flush_cabinet) {
        int state;
        BOOL call_get_next_cab;
        call_get_next_cab = cabx_entries_iter_is_end_of_entry(iter_state)
            ? TRUE : FALSE;
        state = FCIFlushCabinet(iter_state->fci_handle,
            call_get_next_cab,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        result = state ? 0 : -1;
    }

    cabx_i_mem_free(entry_name_w);
    return result;
}

/**
 * You get non zero if entry is last element.
 */
static int
cabx_entries_iter_is_end_of_entry(
    CABX_ENTRY_ITER_STATE* iter_state)
{
    size_t count_of_entries;
    count_of_entries = col_list_size(
        iter_state->generation_status->cabx->entries);
    return iter_state->processed_count >= count_of_entries;
}


/**
 * get non zero if all entries are belong into a cabinet.
 */
static int
cabx_is_generated_all(
    CABX* obj)
{
    return col_list_size(obj->entries) == col_map_size(obj->entry_cab_map);
}


/**
 * encode  string
 */
static int
cabx_encode_str(
    const char* src,
    char** encoded,
    int* attribute)
{
    size_t src_len;
    unsigned short* utf16_str;
    buffer_char_buffer* buffer;
    int result;
    int utf_attr;
    utf_attr = 0;
    src_len = strlen(src);
    utf16_str = (unsigned short*)str_conv_utf8_to_utf16(
        src, src_len + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);
    result = utf16_str ? 0 : -1;
    buffer = NULL;
    if (result == 0) {
        buffer = buffer_char_buffer_create(10);
        result = buffer ? 0 : -1;
    }

    if (result == 0) {
        unsigned short* str_ptr; 
        str_ptr = utf16_str;
        while (*str_ptr != 0) {
            if (0 <= *str_ptr && *str_ptr <= 0x007f) {
                char tmp_buff[] = { (char)(*str_ptr) };
                result = buffer_char_buffer_append(
                    buffer, tmp_buff, sizeof(tmp_buff));
            } else if (0x0080 <= *str_ptr && *str_ptr <= 0x07ff) {
                char tmp_buff[] = { 
                    (char)(0xc0 + ((*str_ptr) >> 6)),
                    (char)(0x80 + ((*str_ptr) & 0x003f))
                };
                result = buffer_char_buffer_append(
                    buffer, tmp_buff, sizeof(tmp_buff));
                utf_attr = 1; 
            } else if (0x0800 <= *str_ptr && *str_ptr <= 0xffff) {
                char tmp_buff[] = { 
                    (char)(0xe0 + ((*str_ptr) >> 12)),
                    (char)(0x80 + ((((*str_ptr) >> 6)) & 0x003f)),
                    (char)(0x80 + ((*str_ptr) & 0x003f))
                };
                result = buffer_char_buffer_append(
                    buffer, tmp_buff, sizeof(tmp_buff));
                utf_attr = 1;
            }
            if (result) {
                break;
            }
            str_ptr++;
        }
        if (result == 0) {
            char null_byte[] = { 0 };
            result = buffer_char_buffer_append(
                buffer, null_byte, sizeof(null_byte));
        }
    }
    if (result == 0) {
        char* res_str;
        res_str = (char*)cabx_i_mem_alloc(
            buffer_char_buffer_get_size(buffer));
        if (res_str) {
            buffer_char_buffer_copy_to(buffer, res_str);
            *encoded = res_str; 
            *attribute = utf_attr ? _A_NAME_IS_UTF : 0;
        } else {
            result = -1;
        }
    }

    if (buffer) {
        buffer_char_buffer_release(buffer);
    }
    return result;
}

/**
 * decode string
 */
static int
cabx_decode_str(
    const char* src,
    char** decoded)
{
    char* utf8_str;
    buffer_variable_buffer* buffer;
    int result;
    buffer = buffer_variable_buffer_create(10, sizeof(uint16_t));
    result = buffer ? 0 : -1;

    if (result == 0) {
        const char* str_ptr; 
        str_ptr = src;
        while (*str_ptr != '\0') {
            uint16_t a_char;
            a_char = 0;
            if ((*str_ptr & 0xf0) == 0xe0) {
                uint16_t tmp_buff;
                tmp_buff = *str_ptr;
                tmp_buff = (uint16_t)((0x0f & tmp_buff) << 12);
                a_char |= tmp_buff;
                str_ptr++;
                tmp_buff = *str_ptr;
                tmp_buff = (uint16_t)((0x3f & tmp_buff) << 6);
                a_char |= tmp_buff;
                str_ptr++;
                tmp_buff = *str_ptr;
                a_char |= (uint16_t)((0x3f & tmp_buff));
            } else if ((*str_ptr & 0xf0) == 0xc0) {
                uint16_t tmp_buff;
                a_char = 0;
                tmp_buff = *str_ptr;
                tmp_buff = (uint16_t)((0x3f & tmp_buff) << 6);
                a_char |= tmp_buff;
                str_ptr++;
                tmp_buff = *str_ptr;
                tmp_buff = (uint16_t)(0x3f & tmp_buff);
                a_char |= tmp_buff;
            } else {
                a_char = *str_ptr;
            }
            result = buffer_variable_buffer_append(buffer, &a_char, 1);

            if (result) {
                break;
            }
            str_ptr++;
        }
        if (result == 0) {
            uint16_t null_char = 0;
            result = buffer_variable_buffer_append(
                buffer, &null_char, 1);
        }
    }

    if (result == 0) {
        utf8_str = str_conv_utf16_to_utf8(
            buffer_variable_buffer_get_data(buffer),
            buffer_variable_buffer_get_size(buffer),
            cabx_i_mem_alloc, cabx_i_mem_free);
        result = utf8_str ? 0 : -1;
    }
    if (result == 0) {
        *decoded = utf8_str;
        utf8_str = NULL;
    }
    if (utf8_str) {
        cabx_i_mem_free(utf8_str);
    }
    
    if (buffer) {
        buffer_variable_buffer_release(buffer);
    }
    return result;
}


/**
 * set last file entry 
 */
static int
cabinet_generation_status_set_last_file_entry(
    CABX_GENERATION_STATUS* obj,
    cstr* last_file_entry)
{
    int result;
    result = 0;
    if (last_file_entry != obj->last_file_entry) {
        if (last_file_entry) {
            cstr_retain(last_file_entry);
        }
        if (obj->last_file_entry) {
            cstr_release(obj->last_file_entry);
        }
        obj->last_file_entry = last_file_entry;
    }
    return result;
}


/**
 * set starting cabinet name 
 */
static int
cabinet_generation_status_set_starting_cabinet_name(
    CABX_GENERATION_STATUS* obj,
    cstr* cabinet_name)
{
    int result;
    result = 0;
    if (cabinet_name != obj->starting_cabinet_name) {
        if (cabinet_name) {
            cstr_retain(cabinet_name);
        }
        if (obj->starting_cabinet_name) {
            cstr_release(obj->starting_cabinet_name);
        }
        obj->starting_cabinet_name = cabinet_name;
    }
    return result;
}

/**
 * set next cabinet name
 */
static int
cabinet_generation_status_set_next_cabinet_name(
    CABX_GENERATION_STATUS* obj,
    cstr* next_cabinet_name)
{
    int result;
    result = 0;
    if (next_cabinet_name != obj->next_cabinet_name) {
        if (next_cabinet_name) {
            cstr_retain(next_cabinet_name);
        }
        if (obj->next_cabinet_name) {
            cstr_release(obj->next_cabinet_name);
        }
        obj->next_cabinet_name = next_cabinet_name;
    }
    return result;
}


/**
 * set next disk name 
 */
static int
cabinet_generation_status_set_next_disk_name(
    CABX_GENERATION_STATUS* obj,
    cstr* next_disk_name)
{
    int result;
    result = 0;
    if (next_disk_name != obj->next_disk_name) {
        if (next_disk_name) {
            cstr_retain(next_disk_name);
        }
        if (obj->next_disk_name) {
            cstr_release(obj->next_disk_name);
        }
        obj->next_disk_name = next_disk_name;
    }
    return result;
}



/**
 * create cabinet
 */
static int
cabx_create_cab(
    CABX* obj,
    HFCI fci_hdl,
    CABX_GENERATION_STATUS* generation_status)
{
    int result;
    CABX_ENTRY_ITER_STATE state;

    result = 0;
    memset(&state, 0, sizeof(state));
    state.fci_handle = fci_hdl;
    state.last_compression_type = tcompBAD;
    state.generation_status = generation_status;
    
    result = col_list_forward_iterate(
        obj->entries,
        (int (*)(void*, const void*))cabx_entries_iter,
        &state);

    return result;
}



/**
 * generate cabinet
 */
static int
cabx_generate(
    CABX* obj)
{
    int result;
    HFCI fci_hdl;
    ERF fci_err;
    CABX_GENERATION_STATUS gen_status;
    CCAB cab_param;
    result = 0;
    fci_hdl = NULL;
    memset(&fci_err, 0, sizeof(fci_err));
    memset(&gen_status, 0, sizeof(gen_status));
    result = cabx_load_entries(obj);
    if (result == 0) {
        result = cabx_fill_cab_param(obj, &cab_param);
    }
    if (result == 0) {
        gen_status.cabx = obj;
        gen_status.ccab = &cab_param;
        result = cabx_put_cabinet_output_dir(obj,
            cab_param.szCab,
            cab_param.szCabPath);
    }
    if (result == 0) {
        fci_hdl = FCICreate(&fci_err,
            cabx_fci_file_placed,
            cabx_fci_alloc,
            cabx_fci_free,
            cabx_fci_open,
            cabx_fci_read,
            cabx_fci_write,
            cabx_fci_close,
            cabx_fci_seek,
            cabx_fci_delete,
            cabx_fci_get_temporary_file_name,
            &cab_param,
            &gen_status);
    }
    if (result == 0) {
        result = cabx_create_cab(obj, fci_hdl, &gen_status);
    }

    if (result == 0 && !cabx_is_generated_all(obj)) {
        int state;
        gen_status.end_of_generation = 1;
        state = FCIFlushCabinet(fci_hdl,
            FALSE,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        if (state) {
            result = 0;
        } else {
            if (fci_err.erfOper == FCIERR_CAB_FILE) {
                result = 0;
            } else {
                result = -1;
            }
        }
    }
    cabinet_generation_status_set_last_file_entry(
        &gen_status, NULL);

    cabinet_generation_status_set_starting_cabinet_name(
        &gen_status, NULL);

    cabinet_generation_status_set_next_cabinet_name(
        &gen_status, NULL);

    cabinet_generation_status_set_next_disk_name(
        &gen_status, NULL);



    if (fci_hdl) {
        FCIDestroy(fci_hdl);
    }
    if (result == 0) {
        cabx_report_cab_map(obj); 
    }
    return result;
}

/**
 * remove last cabinet if it is empty
 */
static int
cabx_remove_last_cab_if_empty(
    CABX* obj,
    cstr* out_dir,
    cstr* cab_name)
{
    int result;
    size_t entry_count;
    result = 0;
    entry_count = 0;
    cabx_cab_entries_get_entry_count(
        obj->cab_entries_map, cab_name, &entry_count);
    if (!entry_count) {
        cstr *tmp_path;
        char* raw_out_dir;
        char* raw_cab_name;
        char* raw_cab_path;
        wchar_t* cab_path_w;
        wchar_t* out_dir_w;
        raw_out_dir = NULL;
        raw_cab_name = NULL;
        raw_cab_path = NULL;
        cab_path_w = NULL;
        out_dir_w = NULL;
        tmp_path = cstr_create_01(
            (void* (*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        result = tmp_path ? 0 : -1;  
        if (result == 0) {
            raw_out_dir = cstr_to_flat_str(out_dir);
            result = raw_out_dir ? 0 : -1;
        }
        if (result == 0) {
            raw_cab_name = cstr_to_flat_str(cab_name);
            result = raw_cab_name ? 0 : -1;
        }
        if (result == 0) {
            result = cstr_append(tmp_path, raw_out_dir);
        }
        if (result == 0) {
            result = cstr_append(tmp_path, raw_cab_name);
        }
        if (result == 0) {
            raw_cab_path = cstr_to_flat_str(tmp_path);
            result = raw_cab_path ? 0 : -1;
        }
        if (result == 0) {
            cab_path_w = (wchar_t*)str_conv_utf8_to_utf16(
                raw_cab_path, cstr_length(tmp_path) + 1,
                cabx_i_mem_alloc, cabx_i_mem_free);
            result = cab_path_w ? 0 : -1;
        }
        if (result == 0) {
            out_dir_w = (wchar_t*)str_conv_utf8_to_utf16(
                raw_out_dir, cstr_length(out_dir) + 1,
                cabx_i_mem_alloc, cabx_i_mem_free);
            result = out_dir_w ? 0 : -1;
        }
        
        if (result == 0) {
            _wremove(cab_path_w);
            _wrmdir(out_dir_w);
        } 
        
        cabx_i_mem_free(out_dir_w);
        cabx_i_mem_free(cab_path_w);
        
        if (result == 0) {
            cstr_free_flat_str(tmp_path, raw_cab_path);
        }
        if (raw_cab_name) {
            cstr_free_flat_str(cab_name, raw_cab_name);
        }  
        if (raw_out_dir) {
            cstr_free_flat_str(out_dir, raw_out_dir);
        }
        if (tmp_path) {
            cstr_release(tmp_path);
        }
    }
    return result;
}



/**
 * fill fci cab parameter
 */
static int
cabx_fill_cab_param(
    CABX* obj,
    CCAB* param)
{
    int result;
    result = 0;

    memset(param, 0, sizeof(*param));

    param->cb = obj->option->max_cabinet_size;
    param->cbFolderThresh = obj->option->folder_threshold; 
    param->iCab = 0;

    cabx_fill_cabinet_name(obj, 
        param->iCab,
        param->szCab,
        sizeof(param->szCab));

    cabx_fill_disk_name(obj, 
        param->iDisk,
        param->szDisk,
        sizeof(param->szDisk));

    cabx_fill_cab_path(
        obj,
        param->szCab,
        param->szCabPath,
        sizeof(param->szCabPath));


    param->setID = (int)rand();
    return result;
}

/**
 * report entry name cabinet map
 */
static int
cabx_report_cab_map(
    CABX* obj)
{
    int result;
    result = 0;
    if (obj->option->report_file) {
        FILE* fs;
        fs = NULL;

        if (strcmp(obj->option->report_file, "-") == 0) {
            fs = stdout;
        } else {
            wchar_t* report_file_w;
            report_file_w = str_conv_utf8_to_utf16(
                obj->option->report_file,
                strlen(obj->option->report_file) + 1,
                cabx_i_mem_alloc, cabx_i_mem_free);
            result = report_file_w ? 0 : -1;
            if (result == 0) {
                fs = _wfopen(report_file_w, L"w");
                result = fs ? 0 : -1;
            }

            if (report_file_w) {
                cabx_i_mem_free(report_file_w);
            }
        }
        if (result == 0) {
            if (obj->option->show_status) {
                if (_isatty(_fileno(stderr)) && _isatty(_fileno(fs))) {
                    fputws(L"\033[K", fs);
                }
            }
        }


        if (result == 0) {
            CABX_ENTRY_ITER_REPORT_STATE rep_state;
            memset(&rep_state, 0, sizeof(rep_state));
            rep_state.cabx = obj;
            rep_state.output_stream = fs;
            result = col_list_forward_iterate(
                obj->entries,
                (int (*)(void*, const void*))cabx_entries_iter_for_reporting,
                &rep_state);
        }


        if (fs && fs != stdout) {
            fclose(fs);
        }
    }
    return result;
}

/**
 * add entry into cab name to entries map
 */
static int
cabx_cab_entries_add_entry(
    col_map* cab_entries_map,
    cstr* cab_name,
    cstr* entry_name)
{
    int result;
    col_list_ref* entries;
    result = 0;
    entries = NULL;
    col_map_get(cab_entries_map, cab_name, (void**)&entries);
    if (!entries) {
        col_list* entries_list;
        entries_list = col_array_list_create(10, 10,
            (int (*)(void*))cstr_hash,
            (int (*)(const void*, void**))cstr_retain_1,
            (void (*)(void*))cstr_release_1);

        result = entries_list ? 0 : -1;
        if (result == 0) {
            entries = col_list_ref_create(entries_list,
                (int (*)(const void*, const void*))cstr_compare);
            result = entries ? 0 : -1;
        }
        if (result == 0) {
            result = col_map_put(cab_entries_map, cab_name, entries);
        }
    }
    if (result == 0) {
        result = col_list_ref_append(entries, entry_name);
    }

    if (entries) {
        col_list_ref_release(entries);
    }
    return result;
}

/**
 * get entry count of cab name to entries map
 */
static int
cabx_cab_entries_get_entry_count(
    col_map* cab_entries_map,
    cstr* cab_name,
    size_t* count)
{
    int result;
    col_list_ref* entries;
    result = 0;
    entries = NULL;
    col_map_get(cab_entries_map, cab_name, (void**)&entries);
    if (entries) {
        *count = col_list_ref_size(entries); 
    } else {
        result = -1;
    }
    if (entries) {
        col_list_ref_release(entries);
    }
    return result;
}



/**
 * entry iterator
 */
static int
cabx_entries_iter_for_reporting(
    CABX_ENTRY_ITER_REPORT_STATE* iter_state,
    CABX_ENTRY* entry)
{
    int result;
    int state;
    cstr* entry_name_cstr; 
    cstr* cabinet_name_cstr;
    wchar_t* entry_name_w;
    wchar_t* cabinet_name_w; 
    result = 0;
    state = 0;
    cabinet_name_cstr = NULL;
    entry_name_w = NULL;
    cabinet_name_w = NULL;
    entry_name_cstr = cstr_create_00(
        entry->entry_name,
        strlen(entry->entry_name),
        (void *(*)(unsigned int))cabx_i_mem_alloc,
        cabx_i_mem_free);
    state = entry_name_cstr ? 0 : -1;
    if (state == 0) {
        col_map_get(
            iter_state->cabx->entry_cab_map,  entry_name_cstr,
            (void **)&cabinet_name_cstr);

        if (cabinet_name_cstr) {
            char* cabinet_name;

            cabinet_name = cstr_to_flat_str_0(cabinet_name_cstr,
                (void* (*)(unsigned int))cabx_i_mem_alloc);  
            state = cabinet_name ? 0 : -1;

            if (state == 0) {
                cabinet_name_w = (wchar_t *)str_conv_utf8_to_utf16(
                    cabinet_name,
                    cstr_length(cabinet_name_cstr) + 1,
                    cabx_i_mem_alloc,
                    cabx_i_mem_free);
                state = cabinet_name_w ? 0 : -1;
            }

            if (cabinet_name) {
                cabx_i_mem_free(cabinet_name);
            }
        }
    }
    if (state == 0) {
        entry_name_w = (wchar_t *)str_conv_utf8_to_utf16(
            entry->entry_name,
            strlen(entry->entry_name) + 1,
            cabx_i_mem_alloc,
            cabx_i_mem_free);
        state = entry_name_w ? 0 : -1;
    }
    if (state == 0) {
        if (entry_name_w && cabinet_name_w) {
            fwprintf(iter_state->output_stream,
                L"%ls,%ls\n",
                entry_name_w,
                cabinet_name_w);
        }
    }


    if (entry_name_w) {
        cabx_i_mem_free(entry_name_w);
    }

    if (cabinet_name_w) {
        cabx_i_mem_free(cabinet_name_w);
    }


    if (cabinet_name_cstr) {
        cstr_release(cabinet_name_cstr);
    }

    if (entry_name_cstr) {
        cstr_release(entry_name_cstr);
    }
}



/**
 * fill cabinet name
 */
static int
cabx_fill_cabinet_name(
    CABX* obj,
    int cab_index,
    char* buffer,
    size_t buffer_size)
{
    int result;
    result = 0;
    if (obj) {
        snprintf(
            buffer, buffer_size,
            obj->option->cabinet_name,
            cab_index);
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}

/**
 * fill disk name
 */
static int
cabx_fill_disk_name(
    CABX* obj,
    int disk_index,
    char* buffer,
    size_t buffer_size)
{
    int result;
    result = 0;
    if (obj) {
        snprintf(
            buffer, buffer_size,
            obj->option->disk_name,
            disk_index);
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}

/**
 * copy cab path into buffer 
 */
static int
cabx_fill_cab_path(
    CABX* obj,
    const char* cab_name,
    char* buffer,
    size_t buffer_size)
{
    int result;
    result = 0;
    if (obj) {
        char* output_dir;
        output_dir = NULL;
        result = path_append_dir_separator(obj->option->output_dir,
            &output_dir, cabx_i_mem_alloc, cabx_i_mem_free);
        
        if (result == 0) {
            strncpy(buffer, output_dir, buffer_size);    
        }
        if (output_dir) {
            cabx_i_mem_free(output_dir);
        }
    } else {
        result = -1;
        errno = EINVAL;
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
 * create option instance
 */
static CABX_OPTION*
cabx_option_create()
{
    CABX_OPTION* result;
    char* input;
    char* output_dir;
    char* cabinet_name;
    char* disk_name;
    result = (CABX_OPTION*)cabx_i_mem_alloc(sizeof(CABX_OPTION));
    input = cabx_i_str_dup("-");
    output_dir = cabx_i_str_dup(".\\");
    cabinet_name = cabx_i_str_dup("data%d.cab");
    disk_name = cabx_i_str_dup("");
    if (result && input && output_dir && cabinet_name && disk_name) {
        result->show_status = 0;
        result->input = input;
        result->output_dir = output_dir;
        result->cabinet_name = cabinet_name;
        result->disk_name = disk_name;
        result->max_cabinet_size = CABX_MAX_CABINET_SIZE_DEF;
        result->folder_threshold = CABX_FOLDER_THRESHOLD_DEF;
        result->report_file = NULL;
    } else {
        if (input) {
            cabx_i_mem_free(input);
        }
        if (output_dir) {
            cabx_i_mem_free(output_dir);
        }
        if (cabinet_name) {
            cabx_i_mem_free(cabinet_name);
        }
        if (disk_name) {
            cabx_i_mem_free(disk_name);
        }
        if (result) {
            cabx_i_mem_free(result);
            result = NULL;
        }
    }
    return result;
}

/**
 * free option instance
 */
static void
cabx_option_free(
    CABX_OPTION* opt)
{
    if (opt) { 
        cabx_option_set_input(opt, NULL);
        cabx_option_set_output_dir(opt, NULL);
        cabx_option_set_cabinet_name(opt, NULL);
        cabx_option_set_disk_name(opt, NULL);
        if (opt->report_file) {
            cabx_i_mem_free(opt->report_file);
            opt->report_file = NULL;
        }
        cabx_i_mem_free(opt);
    }
}


/**
 * set input into option
 */
static int
cabx_option_set_input(
    CABX_OPTION* opt,
    const char* input)
{
    int result;
    result = 0;
    if (opt) {
        if (opt->input != input) {
            if (opt->input) {
                cabx_i_mem_free(opt->input);
                opt->input = NULL;
            }
            if (input) {
                opt->input = cabx_i_str_dup(input);
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}

/**
 * set output into option
 */
static int
cabx_option_set_output_dir(
    CABX_OPTION* opt,
    const char* output_dir)
{
    int result;
    result = 0;
    if (opt) {
        if (opt->output_dir != output_dir) {
            if (opt->output_dir) {
                cabx_i_mem_free(opt->output_dir);
                opt->output_dir = NULL;
            }
            if (output_dir) {
                opt->output_dir = cabx_i_str_dup(output_dir);
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}


/**
 * set cabinet name into option
 */
static int
cabx_option_set_cabinet_name(
    CABX_OPTION* opt,
    const char* cabinet_name)
{
    int result;
    result = 0;
    if (opt) {
        if (opt->cabinet_name != cabinet_name) {
            if (opt->cabinet_name) {
                cabx_i_mem_free(opt->cabinet_name);
                opt->cabinet_name = NULL;
            }
            if (cabinet_name) {
                opt->cabinet_name = cabx_i_str_dup(cabinet_name);
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}

/**
 * set disk name into option
 */
static int
cabx_option_set_disk_name(
    CABX_OPTION* opt,
    const char* disk_name)
{
    int result;
    result = 0;
    if (opt) {
        if (opt->disk_name != disk_name) {
            if (opt->disk_name) {
                cabx_i_mem_free(opt->disk_name);
                opt->disk_name = NULL;
            }
            if (disk_name) {
                opt->disk_name = cabx_i_str_dup(disk_name);
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}

/**
 * set max cabinet size by string format into option
 */
static int
cabx_option_set_max_cabinet_size(
    CABX_OPTION* opt,
    const char* max_size_str)
{
    int result;
    long l_value;
    l_value = 0;
    result = number_parser_str_to_long_with_mod(max_size_str, 0, &l_value);
    if (result == 0) {
        opt->max_cabinet_size = l_value;
    }

    return result;  
}

/**
 * set cabinet folder threshold size by string format into option
 */
static int
cabx_option_set_folder_threshold(
    CABX_OPTION* opt,
    const char* threshold)
{
    int result;
    long l_value;
    l_value = 0;
    result = number_parser_str_to_long_with_mod(threshold, 0, &l_value);
    if (result == 0) {
        opt->folder_threshold = l_value;
    }
    return result;  
}


/**
 * set report file into option
 */
static int
cabx_option_set_report(
    CABX_OPTION* opt,
    const char* report_file)
{
    int result;
    result = 0;
    if (opt) {
        if (!report_file) {
            report_file = "-";
        }
        if (opt->report_file != report_file) {
            if (opt->report_file) {
                cabx_i_mem_free(opt->report_file);
                opt->report_file = NULL;
            }
            opt->report_file = cabx_i_str_dup(report_file);
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}



/**
 * cab entry
 */
static CABX_ENTRY*
cabx_entry_create_0()
{
    CABX_ENTRY* result;
    result = (CABX_ENTRY*)cabx_i_mem_alloc(sizeof(CABX_ENTRY));

    if (result) {
        result->ref_count = 1;
        result->source_file = NULL;
        result->entry_name = NULL;
        result->compression = 0;
        result->attribute = 0;
    }
    return result;
}

/**
 * cab entry
 */
static CABX_ENTRY*
cabx_entry_create_1(
    const char* source_path,
    const char* entry_name,
    int compression,
    int attribute,
    int execute,
    int flush_folder,
    int flush_cabinet)
{
    CABX_ENTRY* result;
    result = cabx_entry_create_0();
    if (result) {
        int state;
        state = cabx_entry_set_source_file(result, source_path);
        if (state == 0) {
            state = cabx_entry_set_entry_name(result, entry_name);
        }
        if (state == 0) {
            result->compression = compression;
        }
        if (state == 0) {
            result->attribute = attribute;
        }
        if (state == 0) {
            result->execute = execute;
        }
        if (state == 0) {
            result->flush_folder = flush_folder;
        }
        if (state == 0) {
            result->flush_cabinet = flush_cabinet;
        }
        if (state) {
            cabx_entry_release(result);
            result = NULL;
        }
    }
    return result;
}


/**
 * increment reference count
 */
static unsigned int
cabx_entry_retain(
    CABX_ENTRY* entry)
{
    unsigned int result;
    result = 0;
    if (entry) {
        result = ++entry->ref_count;
    } else {
        errno = EINVAL;
    }
    return result;
}
   
/**
 * decrement reference count
 */
static unsigned int
cabx_entry_release(
    CABX_ENTRY* entry)
{
    unsigned int result;
    result = 0;
    if (entry) {
        result = --entry->ref_count;
        if (result == 0) {
            cabx_entry_set_source_file(entry, NULL);
            cabx_entry_set_entry_name(entry, NULL);
            cabx_i_mem_free(entry);
        }
    } else {
        errno = EINVAL;
    }
    return result;
}

/**
 * decrement reference count
 */
static void
cabx_entry_release_1(
    CABX_ENTRY* entry)
{
    cabx_entry_release(entry);
}


/**
 * copy entry object
 */
static int
cabx_entry_copy_ref(
    CABX_ENTRY* src_entry,
    CABX_ENTRY** dst_entry)
{
    int result;
    result = 0;
    if (dst_entry && src_entry) {
        cabx_entry_retain(src_entry);
    }
    if (dst_entry) {
        *dst_entry = src_entry;
    }

    return result;
}

/**
 * calculate hash code
 */
static int
cabx_entry_hash_code(
    CABX_ENTRY* entry)
{
    union {
        CABX_ENTRY* obj;
        int hash;
    } pointer_int;
    memset(&pointer_int, 0, sizeof(pointer_int));
    pointer_int.obj = entry;
    return pointer_int.hash;
}

/**
 * set source file
 */
int
cabx_entry_set_source_file(
    CABX_ENTRY* entry,
    const char* source_file)
{
    int result;
    result = 0;
    if (entry)  {
        if (entry->source_file != source_file) {
            char* new_value;
            if (source_file) {
                new_value = cabx_i_str_dup(source_file);
                result = new_value ? 0 : -1;
            } else {
                new_value = 0;
            }
            if (result == 0) {
                if (entry->source_file) {
                    cabx_i_mem_free(entry->source_file);
                    entry->source_file = NULL;
                }
                entry->source_file = new_value;
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}

/**
 * set entry name
 */
static int
cabx_entry_set_entry_name(
    CABX_ENTRY* entry,
    const char* entry_name)
{
    int result;
    result = 0;
    if (entry)  {
        if (entry->entry_name != entry_name) {
            char* new_value;
            if (entry_name) {
                new_value = cabx_i_str_dup(entry_name);
                result = new_value ? 0 : -1;
            } else {
                new_value = 0;
            }
            if (result == 0) {
                if (entry->entry_name) {
                    cabx_i_mem_free(entry->entry_name);
                    entry->entry_name = NULL;
                }
                entry->entry_name = new_value;
            }
        }
    } else {
        errno = EINVAL;
        result = -1;
    }
    return result;
}


/**
 * be called when a file is placed in cabinet.
 */
static int
cabx_fci_file_placed(
    PCCAB pccab,
    char* file,
    long file_size,
    BOOL file_continuation,
    void* user_data)
{
    int result;
    int state;
    char* decode_file;
    cstr* file_entry_cstr;
    cstr* cab_name_cstr;
    wchar_t* decode_file_w;
    wchar_t* cab_name_w;

    CABX_GENERATION_STATUS* gen_status;

    result = 0;
    file_entry_cstr = NULL;
    cab_name_cstr = NULL;
    decode_file = NULL;
    decode_file_w = NULL;
    cab_name_w = NULL;
    gen_status = (CABX_GENERATION_STATUS*)user_data;
    cabx_decode_str(file, &decode_file);
    state = decode_file ? 0 : -1;
    if (state == 0) {
        file_entry_cstr = cstr_create_00(
            decode_file, strlen(decode_file),
            (void* (*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        state = file_entry_cstr ? 0 : -1;
    }
    if (state == 0) {
        cab_name_cstr = cstr_create_00(
            pccab->szCab, strlen(pccab->szCab),
            (void* (*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        state = cab_name_cstr ? 0 : -1;
    }
    if (gen_status->last_file_entry) {
        if (!cstr_compare(file_entry_cstr, gen_status->last_file_entry)) {
            state = col_map_put(
                gen_status->cabx->entry_cab_map,
                gen_status->last_file_entry,
                gen_status->starting_cabinet_name);
            cabinet_generation_status_set_last_file_entry(
                gen_status, NULL);
            cabinet_generation_status_set_starting_cabinet_name(
                gen_status, NULL);
        }
    }
    if (state == 0) {
        if (file_continuation && !gen_status->last_file_entry) {
            state = cabinet_generation_status_set_last_file_entry(
                gen_status, file_entry_cstr);
            if (state == 0) {
                state = cabinet_generation_status_set_starting_cabinet_name(
                    gen_status, cab_name_cstr);
            }
        } else if (!file_continuation) {
            state = col_map_put(
                gen_status->cabx->entry_cab_map,
                file_entry_cstr, cab_name_cstr);

        }

    }
    if (state == 0) {
        state = cabx_cab_entries_add_entry(
            gen_status->cabx->cab_entries_map,
            cab_name_cstr, file_entry_cstr);
    }
    if (state == 0) {
        decode_file_w = str_conv_utf8_to_utf16(
            decode_file, strlen(decode_file) + 1,
            cabx_i_mem_alloc, cabx_i_mem_free);
        state = decode_file_w ? 0 : -1;
    }
    if (state == 0) {
        cab_name_w = (wchar_t*)str_conv_utf8_to_utf16(
           pccab->szCab, 
           strlen(pccab->szCab) + 1,
           cabx_i_mem_alloc, cabx_i_mem_free);
        state = cab_name_w ? 0 : -1;
    }
    if (state == 0) {
        if (file_entry_cstr && cab_name_cstr) {
            state = col_map_put(
                gen_status->cabx->entry_cab_map,
                file_entry_cstr, cab_name_cstr);
        }
    }
    if (state == 0) {
        if (gen_status->cabx->option->show_status) {
            const wchar_t* format;
            if (_isatty(_fileno(stderr))) {
                format = L"\033[Kplaced %ls in %ls\r";
            } else {
                format = L"placed %ls in %ls\n";
            }
            fwprintf(stderr, format,
                decode_file_w, cab_name_w);
        }
    }
    if (file_entry_cstr) {
        cstr_release(file_entry_cstr);
    }
    if (cab_name_cstr) {
        cstr_release(cab_name_cstr);
    }
    if (cab_name_w) {
        cabx_i_mem_free(cab_name_w);
    }
    if (decode_file_w) {
        cabx_i_mem_free(decode_file_w);
    }
    if (decode_file) {
        cabx_i_mem_free(decode_file);
    }

    return result;
}


/**
 * allocate memory for fci
 */
static void* 
cabx_fci_alloc(unsigned long size)
{
    void* result;
    result = cabx_i_mem_alloc(size);
    return result;
}

/**
 * free memory for fci
 */
static void 
cabx_fci_free(void* heap_obj)
{
    cabx_i_mem_free(heap_obj);
}

/**
 * open file for fci
 */
static intptr_t 
cabx_fci_open(
    LPSTR file_path,
    int open_flag,
    int mode,
    int *err,
    void* user_data)
{
    FILE* fs;
    wchar_t* file_path_w;
    int state;
    CABX_GENERATION_STATUS* gen_status;

    fs = NULL;
    file_path_w = NULL;
    gen_status = (CABX_GENERATION_STATUS*)user_data;
    
    state = cabx_handle_file_path_for_output_dir(
        gen_status->cabx, file_path, 
        cabx_create_output_dir_if_not);

    if (state == 0) {
        file_path_w = str_conv_utf8_to_utf16(
            file_path, strlen(file_path) + 1, 
            cabx_i_mem_alloc, cabx_i_mem_free);
        state = file_path_w ? 0 : -1;
    }
    if (state == 0) {
        int fd;
        fd = _wopen(file_path_w, open_flag, mode); 
        if (fd >= 0) {
            const char* f_mode;
            int r_opt = _O_RDONLY;
            int r_p_opt = _O_RDWR; 
            int w_opt = _O_WRONLY | _O_CREAT | _O_TRUNC;
            int w_p_opt = _O_RDWR | _O_CREAT | _O_TRUNC;
            int a_opt = _O_WRONLY | _O_CREAT | _O_APPEND;
            int a_p_opt = _O_RDWR | _O_CREAT | _O_APPEND;
            
            if ((open_flag & a_p_opt) == a_p_opt) {
                f_mode = "a+";
            } else if ((open_flag & a_opt) == a_opt) {
                f_mode = "a";
            } else if ((open_flag & w_p_opt) == w_p_opt) {
                f_mode = "w+";
            } else if ((open_flag & w_opt) == w_opt) {
                f_mode = "w";
            } else if ((open_flag & r_p_opt) == r_p_opt) {
                f_mode = "r+";
            } else {
                f_mode = "r";
            }
            fs = _fdopen(fd, f_mode);
        }
        if (fs == NULL) {
            *err = errno;
        }
    } else {
        *err = errno;
    }
    if (file_path_w) {
        cabx_i_mem_free(file_path_w);
    }
    return (intptr_t)fs;
}

/**
 *  handle file path to for output directory
 */
static int
cabx_handle_file_path_for_output_dir(
    CABX* obj,
    const char* file_path,
    int (*run)(CABX*, const char*, const char*, const char*))
{
    char* file_name;
    char* dir_path;
    size_t dir_path_len;
    size_t file_name_len;
    int result;
    cstr* file_name_cstr;
    cstr* dir_path_cstr;
    result = 0;
    file_name = NULL;
    dir_path = NULL;
    dir_path_len = 0;
    file_name_len = 0;
    file_name_cstr = NULL;
    dir_path_cstr = NULL;
    path_get_file_spec(
            file_path,
            &file_name,
            cabx_i_mem_alloc,
            cabx_i_mem_free);
    result = file_name ? 0 : -1;
    if (result == 0) {
        dir_path_len = strlen(file_path);
        file_name_len = strlen(file_name);
        dir_path_len -= file_name_len;
        dir_path = (char*)cabx_i_mem_alloc(dir_path_len + 1);
        result = dir_path ? 0 : -1; 
    }
    if (result == 0) {
        memcpy(dir_path, file_path, dir_path_len);
        dir_path[dir_path_len] = '\0';
        file_name_cstr = cstr_create_00(
            file_name, file_name_len,
            (void* (*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        result = file_name_cstr ? 0 : -1;
    }
    if (result == 0) {
        dir_path_cstr = cstr_create_00(
            dir_path, dir_path_len,
            (void* (*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        result = dir_path_cstr ? 0 : -1;
    }
    if (result == 0) {
        cstr* dir_path_cstr_0;
        char* dir_path_0;
        dir_path_cstr_0 = NULL;
        dir_path_0 = NULL;
        col_map_get(obj->cab_outdir_map, file_name_cstr,
            (void**)&dir_path_cstr_0);

        if (dir_path_cstr_0) {
            dir_path_0 = cstr_to_flat_str(dir_path_cstr_0);
        }

        if (dir_path_0) {
            char* file_path_0;
            file_path_0 = NULL;
            result = path_join(dir_path_0, file_name, &file_path_0,
                cabx_i_mem_alloc, cabx_i_mem_free);
            if (file_path_0) {
                if (strcmp(file_path, file_path_0) == 0) {
                    result = run(obj, file_path, dir_path, file_name);
                }
            }
            if (file_path_0) {
                cabx_i_mem_free(file_path_0);
            }
        }

        if (dir_path_0) {
            cstr_free_flat_str(dir_path_cstr_0, dir_path_0);
        }

        if (dir_path_cstr_0) {
            cstr_release(dir_path_cstr_0);
        }
    } 

    if (file_name_cstr) {
        cstr_release(file_name_cstr);
    }
    if (dir_path_cstr) {
        cstr_release(dir_path_cstr);
    }
 
    cabx_i_mem_free(file_name);
    cabx_i_mem_free(dir_path);

    return result;
}

/**
 * create output directory if directry donot exist.
 */
static int
cabx_create_output_dir_if_not(
    CABX* obj,
    const char* file_path,
    const char* output_dir,
    const char* file_name)
{
    int result;
    result = 0;

    wchar_t* output_dir_w; 

    output_dir_w = (wchar_t*)str_conv_utf8_to_utf16(
        output_dir, strlen(output_dir) + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);

    result = output_dir_w ? 0 : -1;
    if (result == 0) {
        int status;
        struct _stat st;
        memset(&st, 0, sizeof(st));
        status = _wstat(output_dir_w, &st);
        if (status) {
            result = _wmkdir(output_dir_w);
        }
    }
    cabx_i_mem_free(output_dir_w);
    return result;
}


/**
 * remove output directory if directry exist.
 */
static int
cabx_remove_output_dir(
    CABX* obj,
    const char* file_path,
    const char* output_dir,
    const char* file_name)
{
    int result;
    result = 0;

    wchar_t* output_dir_w; 

    output_dir_w = (wchar_t*)str_conv_utf8_to_utf16(
        output_dir, strlen(output_dir) + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);

    result = output_dir_w ? 0 : -1;
    if (result == 0) {
        int status;
        struct _stat st;
        memset(&st, 0, sizeof(st));
        status = _wstat(output_dir_w, &st);
        if (status == 0) {
            result = _wrmdir(output_dir_w);
        }
    }
    cabx_i_mem_free(output_dir_w);
    return result;
}



/**
 * read data for fci
 */
static unsigned int
cabx_fci_read(
    intptr_t file_hdl,
    void* buffer,
    unsigned int buffer_size,
    int* err,
    void* user_data)
{
    FILE* fs;
    size_t read_size;
    read_size = 0;
    fs = (FILE*)file_hdl;
    read_size = fread(buffer, 1, buffer_size, fs);

    if (read_size == 0 && feof(fs) == 0) {
        *err = errno;
    }
    return (unsigned int)read_size;
}

/**
 * write data for fci
 */
static unsigned int
cabx_fci_write(
    intptr_t file_hdl,
    void* buffer,
    unsigned int buffer_size,
    int* err,
    void* user_data)
{
    FILE* fs;
    size_t written_size;
    fs = (FILE*)file_hdl;

    written_size = fwrite(buffer, 1, buffer_size, fs);

    if (written_size != buffer_size) {
        *err = errno;
    } else {
        fflush(fs);
    }
    return (unsigned int)written_size;
}

/**
 * seek file for fci
 */
static long  
cabx_fci_seek(
    intptr_t file_hdl,
    long dist,
    int seek_type,
    int *err,
    void* user_data)
{
    FILE* fs;
    long result;
    fs = (FILE*)file_hdl;
    result = -1;
    if (fs) {
        int state;
        state = fseek(fs, dist, seek_type);
        if (state) {
            *err = errno;
        } else {
            result = ftell(fs);
        }
    } else {
        *err = EINVAL;
    }
    return result;
}



/**
 * close file for fci
 */
static int
cabx_fci_close(
    intptr_t file_hdl,
    int *err,
    void* user_data)
{
    FILE* fs;
    int result;
    fs = (FILE*)file_hdl;
    result = 0;
    if (fs) {
        fclose(fs); 
    } else {
        *err = EINVAL;
    }
    return result;
}

/**
 * delete file for fci
 */
static int
cabx_fci_delete(
    LPSTR file_path,
    int *err,
    void* user_data)
{
    int result;
    wchar_t* file_path_w;
    result = 0;
    file_path_w = (wchar_t*)str_conv_utf8_to_utf16(
        file_path, strlen(file_path) + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);

    if (file_path_w)  {
        result = _wremove(file_path_w);
        if (result) {
            *err = errno;
        }
    } else {
        *err = errno;
    }

    cabx_i_mem_free(file_path_w);
    
    return result;
}


/**
 * get temporary file name
 */
static int
cabx_fci_get_temporary_file_name(
    char* temporary_file_path,
    int file_path_size,
    void* user_data)
{
    int result;
    wchar_t* tmp_file_w;
    const size_t tmp_file_w_size = PATH_MAX;
    CABX_GENERATION_STATUS* gen_status = (CABX_GENERATION_STATUS*)user_data;

    tmp_file_w = (wchar_t*)cabx_i_mem_alloc(tmp_file_w_size * sizeof(wchar_t)); 
    result = tmp_file_w ? TRUE : FALSE;
    if (result == TRUE) {
        char* tmp_file;
        size_t tmp_file_w_len;
        _wtmpnam(tmp_file_w);

        tmp_file_w_len = wcslen(tmp_file_w);
        tmp_file = str_conv_utf16_to_utf8(tmp_file_w, tmp_file_w_len + 1,
            cabx_i_mem_alloc, cabx_i_mem_free);
        if (tmp_file) {
            size_t tmp_file_len;
            tmp_file_len = strlen(tmp_file);
            if (file_path_size > tmp_file_len + 1) {
                memcpy(temporary_file_path, tmp_file, tmp_file_len + 1);
                result = TRUE;
            }
            cabx_i_mem_free(tmp_file);
        }
    }
    if (tmp_file_w) {
        cabx_i_mem_free(tmp_file_w);
    }
    return result;
}

/**
 * control next cabinet generating operation
 */
static int
cabx_fci_get_next_cabinet(
    CCAB* cab_param,
    unsigned long previous_cab,
    void* user_data)
{
    int result;
    int status;
    CABX_GENERATION_STATUS* gen_status;
    cstr* cab_name_cstr;
    cstr* output_dir_cstr;
    result = TRUE;
    status = 0;

    cab_name_cstr = NULL;
    output_dir_cstr = NULL;

    gen_status = (CABX_GENERATION_STATUS*)user_data;
    if (!gen_status->end_of_generation) {
        cabx_fill_cabinet_name(
            gen_status->cabx,
            cab_param->iCab,
            cab_param->szCab,
            sizeof(cab_param->szCab));

        cabx_fill_disk_name(
            gen_status->cabx,
            cab_param->iDisk,
            cab_param->szDisk,
            sizeof(cab_param->szDisk));

        cabx_fill_cab_path(
            gen_status->cabx,
            cab_param->szCab,
            cab_param->szCabPath,
            sizeof(cab_param->szCabPath));

        cab_name_cstr = cstr_create_00(
            cab_param->szCab, strlen(cab_param->szCab),
            (void* (*)(unsigned int))cabx_i_mem_alloc, 
            cabx_i_mem_free);
        status = cab_name_cstr ? 0 : -1;


        if (status == 0) {
            output_dir_cstr = cstr_create_00(
                gen_status->cabx->option->output_dir,
                strlen(gen_status->cabx->option->output_dir),
                (void* (*)(unsigned int))cabx_i_mem_alloc, 
                cabx_i_mem_free);
            status = output_dir_cstr ? 0 : -1;
        }
        if (status == 0) {
            status = cabx_put_cabinet_output_dir(
                gen_status->cabx,
                cab_param->szCab,
                cab_param->szCabPath);
        }
        if (status == 0) {
            status = cabinet_generation_status_set_next_cabinet_name(gen_status,
                cab_name_cstr);
        }
        if (status == 0) {
            status = cabinet_generation_status_set_next_disk_name(gen_status,
                output_dir_cstr);
        }

    } else {
        memset(cab_param->szCab, 0, sizeof(cab_param->szCab));
        memset(cab_param->szDisk, 0, sizeof(cab_param->szDisk));
        memset(cab_param->szCabPath, 0, sizeof(cab_param->szCabPath));
    }

    if (cab_name_cstr) {
        cstr_release(cab_name_cstr);
    }
    if (output_dir_cstr) {
        cstr_release(output_dir_cstr);
    }

    return result;
}

/**
 * put cabinet output directory
 */
static int
cabx_put_cabinet_output_dir(
    CABX* obj,
    const char* cabinet_name,
    const char* output_dir)
{
    int result;
    if (cabinet_name && output_dir) {
        cstr* cab_name_cstr;
        cstr* output_dir_cstr;
        result = 0;

        cab_name_cstr = NULL;
        output_dir_cstr = NULL;

        cab_name_cstr = cstr_create_00(
            cabinet_name, strlen(cabinet_name),
            (void* (*)(unsigned int))cabx_i_mem_alloc, 
            cabx_i_mem_free);
        result = cab_name_cstr ? 0 : -1;


        if (result == 0) {
            output_dir_cstr = cstr_create_00(
                output_dir,
                strlen(output_dir),
                (void* (*)(unsigned int))cabx_i_mem_alloc, 
                cabx_i_mem_free);
            result = output_dir_cstr ? 0 : -1;
        }
        if (result == 0) {
            result = col_map_put(
                obj->cab_outdir_map,
                cab_name_cstr, output_dir_cstr);
        }
    
        if (cab_name_cstr) {
            cstr_release(cab_name_cstr);
        }
        if (output_dir_cstr) {
            cstr_release(output_dir_cstr);
        }
    } else {
        result = -1;
        errno = EINVAL;
    }
    return result;
}


/**
 * progress
 */
static long
cabx_fci_progress(
    unsigned int status,
    unsigned long data_1,
    unsigned long data_2,
    void* user_data)
{
    long result;
    CABX_GENERATION_STATUS* gen_status;
    result = 0;
    gen_status = (CABX_GENERATION_STATUS*)user_data;
    
    switch (status) {
        case statusFile:
            break;
        case statusFolder:
            break;
        case statusCabinet:
            break;
    }

    return result;
}

/**
 * get file date time attribute as fat format
 */
static intptr_t
cabx_fci_get_open_info(
    LPSTR file_path,
    unsigned short* date,
    unsigned short* time,
    unsigned short* attr,
    int* err,
    void* user_data)
{
    wchar_t* file_path_w;
    size_t file_path_len;
    struct _stat stat_content;
    CABX_GENERATION_STATUS* gen_status;
    intptr_t result;
    cstr* source_path_cstr;
    cstr* entry_name_cstr;
    char* entry_name;
    int state;
    FILE* fs;
    unsigned short attr_0;

    gen_status = (CABX_GENERATION_STATUS*)user_data;
    entry_name_cstr = NULL;
    source_path_cstr = NULL;
    entry_name = NULL;
    result = -1;
    state = 0;
    attr_0 = 0;
    fs = NULL;
    file_path_len = strlen(file_path);
    file_path_w = (wchar_t*)str_conv_utf8_to_utf16(
        file_path, file_path_len + 1, 
        cabx_i_mem_alloc, cabx_i_mem_free);

    fs = (FILE*)cabx_fci_open(file_path, _O_RDONLY, 0, err, user_data);
   
    state = fs ? 0 : -1; 
    if (state == 0) {
        state = _fstat(_fileno(fs), &stat_content);
    }

    if (state == 0) {
        struct tm* tm_info;
        unsigned short date_0;
        unsigned short time_0;
         
        tm_info = gmtime(&stat_content.st_mtime);

        date_0 = tm_info->tm_mday |
                 (tm_info->tm_mon << 5) |
                 (((tm_info->tm_year + 1900) - 1980) << 9);
        time_0 = tm_info->tm_sec >> 1 |
                 (tm_info->tm_min << 5) |
                 (tm_info->tm_hour << 11);
        *date = date_0;
        *time = time_0;
    }
    if (state == 0) {
        source_path_cstr = cstr_create_00(file_path, file_path_len,
            (void *(*)(unsigned int))cabx_i_mem_alloc,
            cabx_i_mem_free);
        state = source_path_cstr ? 0 : -1;
    }
    if (state == 0) {
        result = (intptr_t)fs;
        fs = NULL;
    }
    if (state == 0) {
        col_map_get(
            gen_status->cabx->source_path_entry_map,
            source_path_cstr, (void**)&entry_name_cstr);
    }
    if (entry_name_cstr) {
        entry_name = cstr_to_flat_str(entry_name_cstr);
        state = entry_name ? 0 : -1;
    } 
    if (entry_name) {
        char* encoded_str;
        int encoded_attr;
        encoded_attr = 0;
        encoded_str = NULL; 
        state = cabx_encode_str(entry_name,
            &encoded_str, &encoded_attr);
        attr_0 |= (unsigned short)encoded_attr; 
    }
    *attr = attr_0;

    if (entry_name) {
        cstr_free_flat_str(entry_name_cstr, entry_name);
    }
    if (entry_name_cstr) {
        cstr_release(entry_name_cstr);
    }
    if (source_path_cstr) {
        cstr_release(source_path_cstr);
    } 
    if (file_path_w) {
        cabx_i_mem_free(file_path_w);
    }
    if (fs) {
        cabx_fci_close((intptr_t)fs, err, user_data);
    }
    return result;
}

/* vi: se ts=4 sw=4 et: */
