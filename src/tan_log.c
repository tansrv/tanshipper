/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


#define TAN_LOG_PATH           "/var/log/tanshipper.log"
#define TAN_MAX_STRERROR_SIZE  256


static tan_fd_t tan_open_log_file(const char *path);
static void tan_insert_errno(char *buf, int err, int *k);
static void tan_insert_lf(char *buf, int *k);
static void tan_write_string_to_stderr(const char *buf, int k);


static tan_fd_t  error;


tan_int_t
tan_log_init()
{
    error = tan_open_log_file(TAN_LOG_PATH);
    if (error == -1)
        return TAN_ERROR;

    return TAN_OK;
}


static tan_fd_t
tan_open_log_file(const char *path)
{
    tan_fd_t  fd;

    fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)
        tan_stderr_error(errno, "open(\"%s\") failed", path);

    return fd;
}


void
tan_log(int err, const char *str, ...)
{
    int        k;
    char       buf[TAN_MAX_STR_SIZE];
    va_list    arg;
    struct tm  tm;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    tan_localtime(&tm);

    k = snprintf(buf, TAN_MAX_STR_SIZE,
                 "[%d/%02d/%02d %02d:%02d:%02d] ",
                 tm.tm_year + 1900, tm.tm_mon + 1,
                 tm.tm_mday, tm.tm_hour,
                 tm.tm_min, tm.tm_sec);

    va_start(arg, str);
    vsnprintf(buf + k, TAN_MAX_STR_SIZE - k, str, arg);
    va_end(arg);

    k = strlen(buf);

    if (err)
        tan_insert_errno(buf, err, &k);

    tan_insert_lf(buf, &k);

    write(error, buf, k);
}


void
tan_stderr_error(int err, const char *str, ...)
{
    int      k;
    char     buf[TAN_MAX_STR_SIZE];
    va_list  arg;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    memcpy(buf, "tanshipper: ", 12);

    k = 12;

    va_start(arg, str);
    vsnprintf(buf + k, TAN_MAX_STR_SIZE - k, str, arg);
    va_end(arg);

    k = strlen(buf);

    if (err)
        tan_insert_errno(buf, err, &k);

    tan_insert_lf(buf, &k);

    tan_write_string_to_stderr(buf, k);
}


static void
tan_insert_errno(char *buf, int err, int *k)
{
    char  errmsg[TAN_MAX_STRERROR_SIZE];

    tan_memzero(errmsg, TAN_MAX_STRERROR_SIZE);

    /* POSIX strerror_r()  */
    if (strerror_r(err, errmsg, TAN_MAX_STRERROR_SIZE))
        return;

    snprintf(buf + *k, TAN_MAX_STR_SIZE - *k,
             " (%d: %s)", err, errmsg);

    *k = strlen(buf);
}


static void
tan_insert_lf(char *buf, int *k)
{
    if (TAN_MAX_STR_SIZE - *k > 1) {
        *(buf + *k) = '\n';
        ++*k;
    } else {
        *(buf + *k - 1) = '\n';
    }
}


static void
tan_write_string_to_stderr(const char *buf, int k)
{
    fwrite(buf, 1, k, stderr);
    fflush(stderr);
}
