/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#define _GNU_SOURCE
#include "tan_core.h"
#include "tanshipper.h"


#define TAN_SHIP_TO  "/usr/local/tanshipper/logs/"

#define TAN_HOSTNAME_SIZE  100
#define TAN_FILENAME_SIZE  50


typedef struct {
    char       hostname[TAN_HOSTNAME_SIZE];
    int        file_count;
} __attribute__ ((packed)) tan_hostinfo_t;


typedef struct {
    char       filename[TAN_FILENAME_SIZE];
    long long  offset;
} __attribute__ ((packed)) tan_fileinfo_t;


tan_vector_t(tid, pthread_t);
tan_vector_t(files, tan_fileinfo_t *);


static void tan_thread_init();
static void *tan_start_shipping(void *arg);

static tan_int_t tan_is_in_allowlist(tan_socket_t fd,
                                     const char *host);

static tan_int_t tan_recv_hostinfo(SSL *ssl, tan_hostinfo_t *hi);
static tan_int_t tan_create_dir(tan_hostinfo_t *hi);

static tan_int_t tan_recv_fileinfo(SSL *ssl, tan_vector_files_t *files,
                                   tan_hostinfo_t *hi);
static void tan_create_empty_file(const char *path);
static void tan_insert_file(tan_vector_files_t *files,
                            const char *name, long long size);

static tan_int_t tan_send_filecount(SSL *ssl, tan_vector_files_t *files);

static void tan_send_fileinfo(SSL *ssl, tan_vector_files_t *files,
                              tan_hostinfo_t *hi);
static tan_int_t tan_recv_file(SSL *ssl, FILE *fp);
static tan_int_t tan_recv_filesize(SSL *ssl, long long *size);

static tan_int_t tan_ssl_read(SSL *ssl, void *buf, int num);
static tan_int_t tan_ssl_write(SSL *ssl, void *buf, int num);


tan_vector_tid_t  tids;


int
main(int argc, char **argv)
{
    if (tan_load_cfg() != TAN_OK)
        return -1;

    if (tan_ssl_init() != TAN_OK)
        return -1;

    if (daemon(1, 1)) {

        tan_stderr_error(errno, "daemon() failed");
        return -1;
    }

    if (tan_log_init() != TAN_OK)
        return -1;

    if (tan_create_pidfile() != TAN_OK)
        return -1;

    tan_thread_init();
}


static void
tan_thread_init()
{
    int        k;
    pthread_t  tid;

    tan_vector_init(tids);

    for (k = 0; k < hosts.size; ++k) {

        if (pthread_create(&tid, NULL, tan_start_shipping, &k)) {

            tan_stderr_error(errno, "pthread_create() failed");
            exit(-1);
        }

        tan_vector_push_back(tids, pthread_t, tid);

        tan_msleep(100);
    }

    for (k = 0; k < hosts.size; ++k)
        pthread_join(tids.vec[k], NULL);
}


static void *
tan_start_shipping(void *arg)
{
    int                 k, num;
    SSL                *ssl;
    tan_socket_t        fd;
    tan_single_host_t  *host;
    tan_hostinfo_t      hi;
    tan_vector_files_t  files;

    num = *(int *)arg;

    for (;;) {

        host = hosts.vec[num];

        fd = tan_connect(host->host, host->port);
        if (fd == -1) {

            tan_stderr_error(0, "%s is not listening on port %d",
                             host->host, host->port);

            exit(-1);
        }

        if (tan_is_in_allowlist(fd, host->host) != TAN_OK) {

            tan_close_socket(fd);
            exit(-1);
        }

        ssl = tan_ssl_connect(fd);
        if (ssl == NULL) {

            tan_close_socket(fd);
            continue;
        }

        tan_vector_init(files);

        if (tan_recv_hostinfo(ssl, &hi) != TAN_OK)
            goto next;

        if (tan_create_dir(&hi) != TAN_OK)
            goto next;

        if (tan_recv_fileinfo(ssl, &files, &hi) != TAN_OK)
            goto next;

        if (tan_send_filecount(ssl, &files) != TAN_OK)
            goto next;

        tan_send_fileinfo(ssl, &files, &hi);

    next:

        if (files.size) {

            for (k = 0; k < files.size; ++k)
                free(files.vec[k]);

            tan_vector_clear(files);
        }

        tan_ssl_close(&ssl, &fd);
        sleep(5);
    }
}


static tan_int_t
tan_is_in_allowlist(tan_socket_t fd, const char *host)
{
    char  buf[2];

    tan_memzero(buf, 2);

    recv(fd, buf, 1, 0);

    if (buf[0] == '1') {

        tan_stderr_error(0, "your ip address is not in allowlist, "
                         "server: %s", host);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_recv_hostinfo(SSL *ssl, tan_hostinfo_t *hi)
{
    tan_int_t  ret;

    ret = tan_ssl_read(ssl, (char *)hi, sizeof(tan_hostinfo_t));
    if (ret != TAN_OK)
        return ret;

    hi->file_count = ntohl(hi->file_count);
    return TAN_OK;
}


static tan_int_t
tan_create_dir(tan_hostinfo_t *hi)
{
    char  path[TAN_PATH_SIZE];

    tan_memzero(path, TAN_PATH_SIZE);

    snprintf(path, TAN_PATH_SIZE,
             "%s%s/", TAN_SHIP_TO, hi->hostname);

    return tan_create_dir_if_not_exist(path);
}


static tan_int_t
tan_recv_fileinfo(SSL *ssl, tan_vector_files_t *files,
                  tan_hostinfo_t *hi)
{
    int             k;
    tan_int_t       ret;
    char            path[TAN_PATH_SIZE];
    long long       size;
    tan_fileinfo_t  fi;

    for (k = 0; k < hi->file_count; ++k) {

        ret = tan_ssl_read(ssl, (char *)&fi,
                           sizeof(tan_fileinfo_t));

        if (ret != TAN_OK)
            return ret;

        tan_memzero(path, TAN_PATH_SIZE);

        snprintf(path, TAN_PATH_SIZE,
                 "%s%s/%s", TAN_SHIP_TO,
                 hi->hostname, fi.filename);

        fi.offset = ntohl(fi.offset);

        if (tan_check_file_exists(path) == -1) {

            if (!fi.offset) {

                tan_create_empty_file(path);
                continue;
            }

            tan_insert_file(files, fi.filename, 0);
            continue;
        }

        size = tan_get_filesize(path);
        if (size == -1)
            return TAN_ERROR;

        if (size < fi.offset)
            tan_insert_file(files, fi.filename, size);
    }

    return TAN_OK;
}


static void
tan_create_empty_file(const char *path)
{
    FILE  *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {

        tan_log(errno, "fopen(\"%s\") failed", path);
        return;
    }

    fclose(fp);
}


static void
tan_insert_file(tan_vector_files_t *files,
                const char *name, long long size)
{
    tan_fileinfo_t  *fi;

    fi = (tan_fileinfo_t *)calloc(1, sizeof(tan_fileinfo_t));
    if (fi == NULL)
        tan_log(errno, "calloc() failed");

    strcpy(fi->filename, name);
    fi->offset = htonl(size);

    tan_vector_push_back(*files, tan_fileinfo_t *, fi);
}


static tan_int_t
tan_send_filecount(SSL *ssl, tan_vector_files_t *files)
{
    int        count;
    tan_int_t  ret;

    count = htonl(files->size);

    ret = tan_ssl_write(ssl, (char *)&count, sizeof(int));
    if (ret != TAN_OK)
        return ret;

    if (!files->size)
        return TAN_ERROR;

    return TAN_OK;
}


static void
tan_send_fileinfo(SSL *ssl, tan_vector_files_t *files,
                  tan_hostinfo_t *hi)
{
    int    k;
    char   path[TAN_PATH_SIZE];
    FILE  *fp;

    for (k = 0; k < files->size; ++k) {

        if (tan_ssl_write(ssl, (char *)files->vec[k],
                          sizeof(tan_fileinfo_t))
            != TAN_OK)
        {
            return;
        }

        files->vec[k]->offset = ntohl(files->vec[k]->offset);

        tan_memzero(path, TAN_PATH_SIZE);

        snprintf(path, TAN_PATH_SIZE,
                 "%s%s/%s", TAN_SHIP_TO,
                 hi->hostname, files->vec[k]->filename);

        if (!files->vec[k]->offset)
            fp = fopen(path, "w");
        else
            fp = fopen(path, "a");

        if (fp == NULL) {

            tan_log(errno, "fopen(\"%s\") failed", path);
            exit(-1);
        }

        if (tan_recv_file(ssl, fp) != TAN_OK) {

            fclose(fp);
            return;
        }

        fclose(fp);
    }
}


static tan_int_t
tan_recv_file(SSL *ssl, FILE *fp)
{
    tan_int_t  ret;
    char       buf[SSL3_RT_MAX_PLAIN_LENGTH + 1];
    long long  size;

    ret = tan_recv_filesize(ssl, &size);
    if (ret != TAN_OK)
        return ret;

    size = ntohl(size);

    tan_memzero(buf, SSL3_RT_MAX_PLAIN_LENGTH + 1);

    while (size > SSL3_RT_MAX_PLAIN_LENGTH) {

        ret = tan_ssl_read(ssl, buf, SSL3_RT_MAX_PLAIN_LENGTH);
        if (ret != TAN_OK)
            return ret;

        fwrite(buf, 1, SSL3_RT_MAX_PLAIN_LENGTH, fp);
        size -= SSL3_RT_MAX_PLAIN_LENGTH;
    }

    ret = tan_ssl_read(ssl, buf, size);
    if (ret != TAN_OK)
        return ret;

    fwrite(buf, 1, size, fp);
    return TAN_OK;
}


static tan_int_t
tan_recv_filesize(SSL *ssl, long long *size)
{
    return tan_ssl_read(ssl, (char *)size, sizeof(long long));
}


static tan_int_t
tan_ssl_read(SSL *ssl, void *buf, int num)
{
    if (SSL_read(ssl, buf, num) > 0)
        return TAN_OK;

    return TAN_ERROR;
}


static tan_int_t
tan_ssl_write(SSL *ssl, void *buf, int num)
{
    if (SSL_write(ssl, buf, num) > 0)
        return TAN_OK;

    return TAN_ERROR;
}
