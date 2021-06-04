/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


void
tan_localtime(struct tm *tm)
{
    time_t  now;

    now = time(NULL);

    localtime_r(&now, tm);
}
