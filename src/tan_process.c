/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


#define TAN_PIDFILE  "/var/run/tanshipper.pid"


tan_int_t
tan_create_pidfile()
{
    char      buf[20];
    tan_fd_t  fd;

    fd = open(TAN_PIDFILE, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {

        tan_stderr_error(errno, "open(\"%s\") failed",
                         TAN_PIDFILE);

        return TAN_ERROR;
    }

    tan_memzero(buf, 20);
    snprintf(buf, 20, "%d", getpid());

    if (write(fd, buf, strlen(buf)) == -1) {

        tan_stderr_error(errno, "write(\"%s\") failed",
                         TAN_PIDFILE);

        tan_close_file(fd);
        return TAN_ERROR;
    }

    tan_close_file(fd);
    return TAN_OK;
}


void
tan_remove_pidfile()
{
    remove(TAN_PIDFILE);
}
