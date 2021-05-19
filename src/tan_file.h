/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_FILE_H
#define TAN_FILE_H


#include "tan_core.h"


typedef int  tan_fd_t;


off_t tan_get_filesize(const char *path);
int tan_check_file_exists(const char *path);
tan_int_t tan_create_dir_if_not_exist(const char *path);


#define tan_close_file(fd)  close(fd)


#endif /* TAN_FILE_H */
