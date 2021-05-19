/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_SOCKET_H
#define TAN_SOCKET_H


#include "tan_core.h"


typedef int  tan_socket_t;


tan_socket_t tan_connect(const char *host, in_port_t port);


#define tan_close_socket(fd)  close(fd)


#endif /* TAN_SOCKET_H */
