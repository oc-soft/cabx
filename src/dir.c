#include "path.h"

#include "dir_i.h"

/**
 * create directory with parent directory
 */
int
dir_mkdir_p(
    const char* dir_path)
{


}

/**
 * create directory
 */
int
dir_mkdir(
    const char* dir_path)
{
    return dir_i_mkdir(dir_path);	
}

/**
 * remove directory
 */
int
dir_rmdir(
    const char* dir_path)
{
    return dir_i_rmdir(dir_path);
}

/* vi: se ts=4 sw=4 et: */
