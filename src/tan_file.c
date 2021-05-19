/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


off_t
tan_get_filesize(const char *path)
{
    struct stat  filestat;

    if (stat(path, &filestat))
        return -1;

    return filestat.st_size;
}


int
tan_check_file_exists(const char *path)
{
    if (access(path, F_OK))
        return -1;

    return 0;
}


tan_int_t
tan_create_dir_if_not_exist(const char *path)
{
    if (tan_check_file_exists(path) == -1) {

        if (mkdir(path, 0700))
            return TAN_ERROR;
    }

    return TAN_OK;
}
