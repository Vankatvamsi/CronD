#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "signals.h"

volatile sig_atomic_t sigchld_received = 0;
volatile sig_atomic_t sigterm_received = 0;
volatile sig_atomic_t sighup_received = 0;
volatile sig_atomic_t sigint_received = 0;

void handle_sigchld(int signal)
{
    (void)signal;

    sigchld_received = 1;
}

void handle_sigterm(int signal)
{
    (void)signal;

    sigterm_received = 1;
}

void handle_sighup(int signal)
{
    (void)signal;

    sighup_received = 1;
}

void handle_sigint(int signal)
{
    (void)signal;

    sigint_received = 1;
}

void signals_init(void)
{
    struct sigaction sa;

    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &sa, NULL);

    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = handle_sighup;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    printf("[Signals] Signal handlers initialized.\n");
    printf("[Signals] SIGCHLD, SIGTERM, SIGHUP, SIGINT registered.\n");
}
