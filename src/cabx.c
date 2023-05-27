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
#include <sys/stat.h>
#include <limits.h>
#include "exe_info.h"
#include "col/array_list.h"
#include "col/rb_map.h"
#include "cstr.h"
#include "csv.h"
#include "buffer/char_buffer.h"
#include "name_compression.h"
#include "number_parser.h"
#include "str_conv.h"
#include "str_hash.h"

/**
 * option for cabinet genertor
 */
typedef struct _CABX_OPTION CABX_OPTION;

/**
 * entry for a cabinet
 */
typedef struct _CABX_ENTRY CABX_ENTRY;

/**
 * processed entries
 */
typedef struct _CABX_PROCESSED CABX_PROCESSED;

/**
 * entry iteration status
 */
typedef struct _CABX_ENTRY_ITER_STATE CABX_ENTRY_ITER_STATE;

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
     * entry name processed map
     */
    col_map* entry_processed_map;

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
 * processed file entry
 */
struct _CABX_PROCESSED  {

    /**
     * reference count
     */
    unsigned int ref_count;
    /**
     * compressed size
     */
    unsigned long compressed_size;

    /**
     * uncompressed size
     */
    unsigned long uncompressed_size;


    /**
     * cabinet name
     */
    char* cabinet_name;

    /**
     * cabinet number
     */
    int cabinet_number;
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
     * count of processed files
     */
    unsigned long count_of_processed_files;

    /**
     * compressed size
     */
    unsigned long compressed_size;

    /**
     * end of generation
     */
    int end_of_generation;


    /**
     * count of files in not flushed folder
     */
    size_t not_flushed_file_count;

    /**
     * entry name
     */
    const char* entry_name;
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
 * entry iterator
 */
static int
cabx_entries_iter(
    CABX_ENTRY_ITER_STATE* iter_state,
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
 * create processed entry
 */
static CABX_PROCESSED*
cabx_processed_create();

/**
 * increment reference count
 */
static unsigned int
cabx_processed_retain(
    CABX_PROCESSED* obj);

/**
 * decrement reference count
 */
static unsigned int
cabx_processed_release(
    CABX_PROCESSED* obj);

/**
 * decrement reference count
 */
static void
cabx_processed_release_1(
    CABX_PROCESSED* obj);



/**
 * copy prcessed entry
 */
static int
cabx_processed_copy_ref(
    CABX_PROCESSED* src,
    CABX_PROCESSED** dst);


/**
 * calculate hash code
 */
static int
cabx_processed_hash_code(
    CABX_PROCESSED* obj);

/**
 * cabinet name
 */
static int
cabx_processed_set_cabinet_name(
    CABX_PROCESSED* obj,
    const char* cabinet_name);


/**
 * encode  string
 */
static int
cabx_encode_str(
    const char* src,
    char** encoded,
    int* attribute);

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
 * create cabinet generator instance
 */
CABX*
cabx_create()
{
    CABX* result;
    CABX_OPTION* option;
    col_list* entries;
    col_map* source_path_entry_map;
    col_map* entry_processed_map;
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
    entry_processed_map = col_rb_map_create(
        (int (*)(const void*, const void*))cstr_compare,
        (unsigned int (*)(const void*))cstr_hash,
        (unsigned int (*)(const void*))cabx_processed_hash_code,
        (int (*)(const void*, void**))cabx_processed_copy_ref,
        (int (*)(const void*, void**))cstr_retain_1,
        (void (*)(void*))cabx_processed_release_1,
        (void (*)(void*))cstr_release_1);



    if (result && option && entries
        && source_path_entry_map
        && entry_processed_map) {
        result->ref_count = 1;
        result->run = cabx_generate;
        result->option = option;
        result->entries = entries;
        result->source_path_entry_map = source_path_entry_map;
        result->entry_processed_map = entry_processed_map;
    } else {
        if (entry_processed_map) {
            col_map_free(entry_processed_map);
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
            col_map_free(obj->entry_processed_map);
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
            "i:o:d:c:m:h", options, NULL);

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
            case 'o':
                result = cabx_option_set_output_dir(obj->option, optarg);
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
"                                   default is %dbytes\n"
"-h                                 show this message\n",
        exe_name, LONG_MAX);


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
    CABX_PROCESSED* processed;
    result = 0;
    encoded_attr = 0;
    encoded_name = NULL;
    processed = NULL;

    if (iter_state->last_compression_type == tcompBAD) {
        iter_state->last_compression_type = entry->compression;
    } else {
        int flush_folder;
        flush_folder = iter_state->last_compression_type != entry->compression;
        if (flush_folder) {
            int state;
            wprintf(L"FCIFlushFolder-1 %ls\n", entry_name_w);
            state = FCIFlushFolder(iter_state->fci_handle,
                cabx_fci_get_next_cabinet,
                cabx_fci_progress);
            result = state ? 0 : -1;
        }
    }

    entry_name_w = str_conv_utf8_to_utf16(
        entry->entry_name, strlen(entry->entry_name) + 1,
        cabx_i_mem_alloc, cabx_i_mem_free);

    result = cabx_encode_str(entry->entry_name, &encoded_name, &encoded_attr);
    if (result == 0) {
        processed = cabx_processed_create();
        result = processed ? 0 : -1;
    }
    if (result == 0) {
        iter_state->generation_status->entry_name = entry->entry_name;
    }
    
    if (result == 0) {
        int state;
        wprintf(L"FCIAddFile %ls\n", entry_name_w);
        state = FCIAddFile(iter_state->fci_handle,
            entry->source_file,
            encoded_name,
            entry->execute ? TRUE : FALSE,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress,
            cabx_fci_get_open_info,
            entry->compression);
        wprintf(L"FCIAddFile-1 %d %ls\n", state, entry_name_w);
        
        result = state ? 0 : -1;
    }
    if (result == 0) {
        iter_state->last_compression_type = entry->compression;
        iter_state->processed_count++;
    }

    if (result == 0 && entry->flush_folder) {
        int state;
        wprintf(L"FCIFlushFolder-2 %ls\n", entry_name_w);
        state = FCIFlushFolder(iter_state->fci_handle,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        result = state ? 0 : -1;
    }
  
    if (result == 0 && entry->flush_cabinet) {
        int state;
        size_t count_of_entries;
        BOOL call_get_next_cab;

        count_of_entries = col_list_size(
            iter_state->generation_status->cabx->entries);
        if (iter_state->processed_count < count_of_entries) {
            call_get_next_cab = TRUE;
        } else {
            iter_state->generation_status->end_of_generation = 1;   
            call_get_next_cab = FALSE;
        }
        wprintf(L"FCIFlushCabinet %ls\n", entry_name_w);
        state = FCIFlushCabinet(iter_state->fci_handle,
            call_get_next_cab,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        result = state ? 0 : -1;
    }

    if (processed) {
        cabx_processed_release(processed);
    }

    cabx_i_mem_free(entry_name_w);
    return result;
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

    if (result == 0 && !gen_status.end_of_generation) {
        int state;
        gen_status.end_of_generation = 1;
        state = FCIFlushCabinet(fci_hdl,
            FALSE,
            cabx_fci_get_next_cabinet,
            cabx_fci_progress);
        result = state ? 0 : -1;
    }
    if (fci_hdl) {
        FCIDestroy(fci_hdl);
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

    param->cb = ULONG_MAX;
    param->cbFolderThresh = 0; 
    param->iCab = 0;

    snprintf(
        param->szCab, sizeof(param->szCab),
        obj->option->cabinet_name,
        param->iCab);
    memcpy(param->szCabPath,
        obj->option->output_dir,
        strlen(obj->option->output_dir) + 1);
    param->setID = (int)rand();
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
        result->input = input;
        result->output_dir = output_dir;
        result->cabinet_name = cabinet_name;
        result->disk_name = disk_name;
        result->max_cabinet_size = LONG_MAX;
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
    char* end_ptr;
    long l_value;
    result = 0;
    end_ptr = NULL;
    l_value = strtol(max_size_str, &end_ptr, 0);

    result = end_ptr != max_size_str ? 0 : -1;

    if (result == 0) {
        char mod;
        mod = *end_ptr;

        if (mod == 'k' || mod == 'K') {
            l_value *= 0x400;
        } else if (mod == 'm' || mod == 'M') {
            l_value *= 0x100000;
        }
        opt->max_cabinet_size = l_value;
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
 * create processed entry
 */
static CABX_PROCESSED*
cabx_processed_create()
{
    CABX_PROCESSED* result;
    result = (CABX_PROCESSED*)cabx_i_mem_alloc(sizeof(CABX_PROCESSED));
    if (result) {
        result->ref_count = 1;
        result->compressed_size = 0;
        result->uncompressed_size = 0;
        result->cabinet_name = NULL;
        result->cabinet_number = 0;
    }
    return result;
}

/**
 * increment reference count
 */
static unsigned int
cabx_processed_retain(
    CABX_PROCESSED* obj)
{
    unsigned int result;
    result = 0;
    if (obj) {
        result = ++obj->ref_count;
    }
    return result;
}

/**
 * decrement reference count
 */
static unsigned int
cabx_processed_release(
    CABX_PROCESSED* obj)
{
    unsigned int result;
    result = 0;
    if (obj) {
        result = --obj->ref_count;
        if (result == 0) {
            cabx_processed_set_cabinet_name(obj, NULL);
            cabx_i_mem_free(obj);
        }
    }
    return result;
}

/**
 * decrement reference count
 */
static void
cabx_processed_release_1(
    CABX_PROCESSED* obj)
{
    cabx_processed_release(obj);
}

/**
 * copy prcessed entry
 */
static int
cabx_processed_copy_ref(
    CABX_PROCESSED* src,
    CABX_PROCESSED** dst)
{
    int result;
    result = 0;
    if (src && dst) {
        cabx_processed_retain(src);
    }
    if (dst) {
        *dst = src;
    }
    return result;
}


/**
 * calculate hash code
 */
static int
cabx_processed_hash_code(
    CABX_PROCESSED* obj)
{
    int result;
    result = 0;
    if (obj) {
        result ^= obj->compressed_size;
        result ^= obj->uncompressed_size;
        result ^= obj->cabinet_number;
        result ^= str_hash_1(obj->cabinet_name); 
    }
    return result;
}

/**
 * cabinet name
 */
static int
cabx_processed_set_cabinet_name(
    CABX_PROCESSED* obj,
    const char* cabinet_name)
{
    int result;
    result = 0;

    if (obj->cabinet_name != cabinet_name) {
        char* tmp_str;
        tmp_str = NULL;
        if (cabinet_name) {
            tmp_str = cabx_i_str_dup(cabinet_name);
            result = tmp_str ? 0 : -1;
        }
        if (result == 0) {
            if (obj->cabinet_name) {
                cabx_i_mem_free(obj->cabinet_name);
            }
            obj->cabinet_name = tmp_str;
        }

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
    result = 0;

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
    fs = NULL;
    file_path_w = str_conv_utf8_to_utf16(
        file_path, strlen(file_path) + 1, 
        cabx_i_mem_alloc, cabx_i_mem_free);
    if (file_path_w) {
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
    CABX_GENERATION_STATUS* gen_status;
    result = FALSE;

    gen_status = (CABX_GENERATION_STATUS*)user_data;

    snprintf(cab_param->szCab,
        sizeof(cab_param->szCab),
        gen_status->cabx->option->cabinet_name,
        cab_param->iCab);

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
            printf("progess status file %u %u \n", data_1, data_2);
            break;
        case statusFolder:
            gen_status->not_flushed_file_count = 0;
            printf("progess status folder %u %u\n", data_1, data_2);
            break;
        case statusCabinet:
            result = LONG_MAX;
            if (gen_status->end_of_generation) {
                printf("progess status cabinet end %u %u\n", data_1, data_2);
            } else {
                printf("progess status cabinet %u %u\n", data_1, data_2);
            }
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
