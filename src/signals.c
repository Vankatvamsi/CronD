#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "signals.h"


volatile sig_atomic_t sigchld_received = 0;
volatile sig_atomic_t sigterm_received = 0;
volatile sig_atomic_t sighup_received = 0;
volatile sig_atomic_t sigint_received = 0;


/*
 * SIGCHLD handler
 */
void handle_sigchld(int signal)
{
    (void)signal;

    sigchld_received = 1;
}


/*
 * SIGTERM handler
 */
void handle_sigterm(int signal)
{
    (void)signal;

    sigterm_received = 1;
}


/*
 * SIGHUP handler
 */
void handle_sighup(int signal)
{
    (void)signal;

    sighup_received = 1;
}


/*
 * SIGINT handler
 */
void handle_sigint(int signal)
{
    (void)signal;

    sigint_received = 1;
}


/*
 * Initialize CronD signal handling.
 */
void signals_init(void)
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART;


    /*
     * SIGCHLD
     */
    sa.sa_handler = handle_sigchld;

    sigaction(
        SIGCHLD,
        &sa,
        NULL
    );


    /*
     * SIGTERM
     */
    sa.sa_handler = handle_sigterm;

    sigaction(
        SIGTERM,
        &sa,
        NULL
    );


    /*
     * SIGHUP
     */
    sa.sa_handler = handle_sighup;

    sigaction(
        SIGHUP,
        &sa,
        NULL
    );


    /*
     * SIGINT
     */
    sa.sa_handler = handle_sigint;

    sigaction(
        SIGINT,
        &sa,
        NULL
    );


    /*
     * CronD should NOT be stopped by Ctrl+Z.
     *
     * Ctrl+Z generates SIGTSTP.
     *
     * CronD ignores it, while the interactive
     * child resets SIGTSTP to SIG_DFL.
     */
    signal(
        SIGTSTP,
        SIG_IGN
    );


    /*
     * Ignore terminal background-control signals.
     *
     * These are important when CronD gives
     * terminal control to another process group.
     */
    signal(
        SIGTTIN,
        SIG_IGN
    );

    signal(
        SIGTTOU,
        SIG_IGN
    );
}


/*
 * Reset signal handlers inside child processes.
 *
 * The child must behave like a normal Linux
 * foreground process.
 */
void reset_child_signal_handlers(void)
{
    /*
     * Restore default behavior.
     */
    signal(
        SIGCHLD,
        SIG_DFL
    );

    signal(
        SIGTERM,
        SIG_DFL
    );

    signal(
        SIGHUP,
        SIG_DFL
    );

    signal(
        SIGINT,
        SIG_DFL
    );

    signal(
        SIGTSTP,
        SIG_DFL
    );

    signal(
        SIGQUIT,
        SIG_DFL
    );


    /*
     * Restore terminal-related signals too.
     */
    signal(
        SIGTTIN,
        SIG_DFL
    );

    signal(
        SIGTTOU,
        SIG_DFL
    );
}
