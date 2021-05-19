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

        printf("open(\"%s\") failed\n", TAN_PIDFILE);
        return TAN_ERROR;
    }

    tan_memzero(buf, 20);
    snprintf(buf, 20, "%d", getpid());

    if (write(fd, buf, strlen(buf)) == -1) {
        printf("write(\"%s\") failed\n", TAN_PIDFILE);

        tan_close_file(fd);
        return TAN_ERROR;
    }

    tan_close_file(fd);
    return TAN_OK;
}
