#include "path.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "buffer/variable_buffer.h"


static int
read_all_inputs(
    char*** lines,
    size_t* line_count,
    FILE* fs);


static void
free_lines(
    size_t line_count,
    char** lines);


static void*
mem_alloc(
    size_t size);


static void
mem_free(
    void* heap_obj);

static int
trim(
    const char* str,
    const char** dst,
    size_t* length);

static int
read_all_inputs(
    char*** lines,
    size_t* count,
    FILE* fs)
{
    char* line_buffer;
    int result;
    buffer_variable_buffer* line_ptrs;
    const static size_t line_buffer_size = 1024;
    result = 0;
    line_ptrs = buffer_variable_buffer_create(10, sizeof(char*));
    result = line_ptrs ? 0 : -1;
    if (result == 0) {
        line_buffer = (char*)malloc(line_buffer_size);
        result = line_buffer ? 0 : -1;
    }
    if (result == 0) {
        while (1) {
            if (fgets(line_buffer, line_buffer_size, fs)) {
                char* line;
                const char* start_ptr;
                size_t length;
                trim(line_buffer, &start_ptr, &length);
                line = (char*)malloc(length + 1);
                result = line ? 0 : -1;
                if (result == 0) {
                    memcpy(line, start_ptr, length);
                    line[length] = '\0';
                    result = buffer_variable_buffer_append(line_ptrs,
                        &line, 1);
                }
            } else {
                break;
            }
            if (result) {
                break;
            }
        }
    }
    if (result == 0) {
        char** lines_0;
        lines_0 = (char**)malloc(
            sizeof(char*) * buffer_variable_buffer_get_size(line_ptrs));
        if (lines_0) {
            buffer_variable_buffer_copy_to(line_ptrs, lines_0);
            *lines = lines_0;
            *count = buffer_variable_buffer_get_size(line_ptrs);
        } else {
            result = -1;
        }
    }

    if (line_ptrs) {
        buffer_variable_buffer_release(line_ptrs);
    }

    if (line_buffer) {
        free(line_buffer);
    }
    return result;
}

static void*
mem_alloc(
    size_t size)
{
    return malloc(size);
}


static void
mem_free(
    void* heap_obj)
{
    free(heap_obj);
}

static int 
test_path_0(
    size_t line_count,
    char** lines)
{

    int result;
    size_t idx;
    result = 0;
    for (idx = 0; idx < line_count; idx++) {
        char* parent_path;
        parent_path = NULL; 
        result = path_remove_file_spec(
            lines[idx],
            &parent_path,
            mem_alloc,
            mem_free);
        printf("%s\n", parent_path);
        free(parent_path);
        if (result) {
            break;
        }
    }
    return result;
}

static int
trim(
    const char* str,
    const char** dst,
    size_t* length)
{
    int result;
    const char* ptr;
    result = 0;
    ptr = str;
    while (*ptr) {
        if (!isspace(*ptr)) {
            break;
        }
        ptr++;
    }
    *dst = ptr;
    if (*ptr) {
        size_t len;
        const char* end_ptr;
        end_ptr = ptr;
        len = strlen(ptr);
        ptr = ptr + len - 1;
        while (*dst != ptr) {
            if (!isspace(*ptr)) {
                end_ptr = ptr + 1;
                break;
            }
            ptr--;
        }
        *length = end_ptr - *dst;
    } else {
        *length = 0;
    }
    return result;
}



static void
free_lines(
    size_t line_count,
    char** lines)
{
    size_t idx;
    for (idx = 0; idx < line_count; idx++) {
        free(lines[idx]);
    }
    free(lines);

}

int
main(
    int argc,
    char** argv)
{
    int result;
    char** lines;
    FILE* fs;
    size_t lines_count;
    int close_fs;
    int idx;

    lines = NULL;
    lines_count = 0;
    close_fs = 0;
    fs = stdin;
    
    for (idx = 1; idx < argc; idx++) {
        if (strcmp("-i", argv[idx]) == 0) {
            if (idx < argc - 1) {
                FILE* tmp_fs;
                idx++;
                tmp_fs = fopen(argv[idx], "r");
                result = tmp_fs ? 0 : -1;
                if (result == 0) {
                    close_fs = 1;
                    fs = tmp_fs;
                }
            } else {
                result = -1;
            }
        }
    }
    if (result == 0) {
        result = read_all_inputs(&lines, &lines_count, fs);
    }
    if (result == 0) {
        result = test_path_0(lines_count, lines);
    }

    if (close_fs) {
        fclose(fs);
    }

    free_lines(lines_count, lines);

    return result;
}


/* vi: se ts=4 sw=4 et: */
