/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_CONFIG_H
#define TAN_CONFIG_H


#include "tan_core.h"


typedef int  tan_int_t;

#define TAN_CFG_PATH  "/etc/tanshipper.conf"

#define TAN_PATH_SIZE      1500
#define TAN_MAX_STR_SIZE   2048
#define TAN_HOSTADDR_SIZE  20


typedef struct {
    char       host[TAN_HOSTADDR_SIZE];
    in_port_t  port;
} tan_single_host_t;


tan_vector_t(hosts, tan_single_host_t *);


tan_int_t tan_load_cfg();


#endif /* TAN_CONFIG_H */
