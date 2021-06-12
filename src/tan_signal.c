/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


static void tan_signal_handler(int signo, siginfo_t *info, void *ctx);


static int  linux_signals[] = {
    SIGTERM, SIGPIPE,
    SIGABRT, 0,
};


tan_int_t
tan_signal_init()
{
    int               k;
    struct sigaction  sa;

    for (k = 0; linux_signals[k]; ++k) {

        tan_memzero(&sa, sizeof(sa));

        if (linux_signals[k] == SIGPIPE ||
            linux_signals[k] == SIGABRT)
        {
            sa.sa_handler = SIG_IGN;
        } else {
            sa.sa_flags     = SA_SIGINFO;
            sa.sa_sigaction = tan_signal_handler;
        }

        sigemptyset(&sa.sa_mask);

        if (sigaction(linux_signals[k], &sa, NULL)) {

            tan_stderr_error(errno, "sigaction() failed");
            return TAN_ERROR;
        }
    }

    return TAN_OK;
}


static void
tan_signal_handler(int signo, siginfo_t *info, void *ctx)
{
    switch (signo) {

    case SIGTERM:
        tan_remove_pidfile();
    }
}
