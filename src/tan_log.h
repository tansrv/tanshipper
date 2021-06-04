/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_LOG_H
#define TAN_LOG_H


#include "tan_core.h"


tan_int_t tan_log_init();
void tan_log(int err, const char *str, ...);
void tan_stderr_error(int err, const char *str, ...);


#endif /* TAN_LOG_H */
