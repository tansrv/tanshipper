/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


tan_socket_t
tan_connect(const char *host, in_port_t port)
{
    int                 k;
    tan_socket_t        fd;
    struct sockaddr_in  addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return fd;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    for (k = 0; k < 5; ++k) {

        if (!connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
            return fd;

        tan_msleep(100);
    }

    close(fd);
    return -1;
}
