/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_OPENSSL_SSL_H
#define TAN_OPENSSL_SSL_H


#include "tan_core.h"

#include <openssl/ssl.h>
#include <openssl/err.h>


tan_int_t tan_ssl_init();
SSL *tan_ssl_connect(tan_socket_t fd);
void tan_ssl_close(SSL **ssl, tan_socket_t *fd);


#endif /* TAN_OPENSSL_SSL_H */
