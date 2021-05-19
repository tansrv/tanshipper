/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


static void tan_remove_last_character(char *str);
static int tan_format_filter(const char *str);
static void tan_load_host(char *str);


tan_vector_hosts_t  hosts;


tan_int_t
tan_load_cfg()
{
    char   str[TAN_MAX_STR_SIZE];
    FILE  *fp;

    fp = fopen(TAN_CFG_PATH, "r");
    if (fp == NULL) {

        printf("fopen(\"%s\") failed\n", TAN_CFG_PATH);
        return TAN_ERROR;
    }

    tan_vector_init(hosts);

    while (!feof(fp)) {

        tan_memzero(str, TAN_MAX_STR_SIZE);

        fgets(str, TAN_MAX_STR_SIZE - 1, fp);

        if (*str == '#')
            continue;

        tan_remove_last_character(str);

        if (tan_format_filter(str) == 1)
            tan_load_host(str);
    }

    fclose(fp);

    if (!hosts.size) {

        printf("tanshipper: configuration file: %s: host not found\n",
               TAN_CFG_PATH);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static void
tan_remove_last_character(char *str)
{
    size_t  len;

    len = strlen(str);

    while (str[len - 1] == '\r' ||
           str[len - 1] == '\n' ||
           str[len - 1] == ';')
    {
        str[--len] = '\0';
    }
}


static int
tan_format_filter(const char *str)
{
    int  k, i, j;

    k = 0, i = 0;

    for (j = 0; str[j]; ++j) {

        if (str[j] == '.')
            ++k;

        if (str[j] == ':')
            ++i;
    }

    if (k == 3 && i == 1)
        return 1;

    return 0;
}


static void
tan_load_host(char *str)
{
    char               *p;
    tan_single_host_t  *sh;

    sh = (tan_single_host_t *)calloc(1, sizeof(tan_single_host_t));
    if (sh == NULL)
        printf("calloc() failed\n");

    p = strchr(str, ':');
    sh->port = atoi(++p);

    strcpy(sh->host, strtok(str, ":"));

    tan_vector_push_back(hosts, tan_single_host_t *, sh);
}
