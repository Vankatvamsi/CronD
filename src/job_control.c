#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include "job_control.h"

/*
 * CronD's original process group.
 *
 * Before starting an interactive child, we save
 * CronD's process group so that terminal control
 * can be returned after the child finishes.
 */
static pid_t crond_pgid = -1;


/*
 * Set the foreground process group of the
 * controlling terminal.
 */
static int set_terminal_foreground(pid_t pgid)
{
    /*
     * If stdin is not a terminal, there is no
     * terminal process group to control.
     */
    if (!isatty(STDIN_FILENO))
    {
        return 0;
    }

    struct sigaction ignore_action;
    struct sigaction old_action;

    /*
     * Temporarily ignore SIGTTOU.
     *
     * SIGTTOU can stop a process when it tries
     * to change terminal settings from a
     * background process group.
     */
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;

    if (sigaction(
            SIGTTOU,
            &ignore_action,
            &old_action
        ) == -1)
    {
        perror("[JobControl] sigaction");
        return -1;
    }

    /*
     * Give terminal control to the requested
     * process group.
     */
    int result = tcsetpgrp(
        STDIN_FILENO,
        pgid
    );

    if (result == -1)
    {
        perror("[JobControl] tcsetpgrp");
    }

    /*
     * Restore the previous SIGTTOU handler.
     */
    sigaction(
        SIGTTOU,
        &old_action,
        NULL
    );

    return result;
}


/*
 * Create a new process group.
 */
int create_process_group(pid_t pid)
{
    if (setpgid(pid, pid) == -1)
    {
        perror("[JobControl] setpgid");
        return -1;
    }

    return 0;
}


/*
 * Terminate an entire process group.
 */
int terminate_process_group(pid_t pgid)
{
    if (kill(-pgid, SIGTERM) == -1)
    {
        perror("[JobControl] kill process group");
        return -1;
    }

    return 0;
}


/*
 * Display process group information.
 */
int get_process_group_info(pid_t pid)
{
    pid_t pgid = getpgid(pid);

    if (pgid == -1)
    {
        perror("[JobControl] getpgid");
        return -1;
    }

    printf(
        "[JobControl] PID  : %d\n",
        (int)pid
    );

    printf(
        "[JobControl] PGID : %d\n",
        (int)pgid
    );

    return 0;
}


/*
 * Give the controlling terminal to an
 * interactive child process group.
 */
int give_terminal_to_process(pid_t pgid)
{
    /*
     * Save CronD's original process group
     * before handing terminal control away.
     */
    if (crond_pgid == -1)
    {
        crond_pgid = getpgrp();
    }

    return set_terminal_foreground(pgid);
}


/*
 * Return terminal control to CronD.
 */
int restore_terminal_to_crond(void)
{
    /*
     * In case this function is called before
     * give_terminal_to_process().
     */
    if (crond_pgid == -1)
    {
        crond_pgid = getpgrp();
    }

    return set_terminal_foreground(
        crond_pgid
    );
}
