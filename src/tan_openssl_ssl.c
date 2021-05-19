/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


static SSL_CTX  *ssl_ctx;


tan_int_t
tan_ssl_init()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ssl_ctx = SSL_CTX_new(SSLv23_method());
    if (ssl_ctx == NULL) {

        ERR_print_errors_fp(stderr);
        return TAN_ERROR;
    }

    return TAN_OK;
}


SSL *
tan_ssl_connect(tan_socket_t fd)
{
    SSL  *ssl;

    ssl = SSL_new(ssl_ctx);
    if (ssl == NULL) {

        printf("SSL_new() failed\n");
        return NULL;
    }

    if (!SSL_set_fd(ssl, fd)) {

        printf("SSL_set_fd() failed\n");
        goto failed;
    }

    if (SSL_connect(ssl) == 1)
        return ssl;

failed:

    SSL_free(ssl);
    return NULL;
}


void
tan_ssl_close(SSL **ssl, tan_socket_t *fd)
{
    if (*ssl != NULL) {
        SSL_shutdown(*ssl);
        SSL_free(*ssl);
        *ssl = NULL;
    }

    if (*fd != -1) {
        tan_close_socket(*fd);
        *fd = -1;
    }
}
