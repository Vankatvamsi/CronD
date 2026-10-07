#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "executor.h"
#include "process.h"
#include "jobs.h"
#include "job_control.h"
#include "signals.h"


/*
 * Execute a command asynchronously.
 *
 * CronD creates a child process and immediately
 * returns to the CronD command prompt.
 */
void execute_job_async(const char *command)
{
    pid_t pid = create_process();

    if (pid == -1)
    {
        printf(
            "[CronD] Failed to create process.\n"
        );

        return;
    }


    /*
     * Child process.
     */
    if (pid == 0)
    {
        /*
         * Create a separate process group.
         */
        if (setpgid(0, 0) == -1)
        {
            perror("[CronD] setpgid");

            _exit(1);
        }

        /*
         * Execute requested command.
         */
        execute_process(command);

        /*
         * Reached only if exec fails.
         */
        _exit(127);
    }


    /*
     * Parent also attempts to place the child
     * into its own process group.
     */
    setpgid(pid, pid);


    /*
     * Add process to CronD job table.
     */
    int job_id = job_add(
        pid,
        command
    );

    if (job_id == -1)
    {
        printf(
            "[CronD] Job table is full.\n"
        );

        terminate_process(pid);

        return;
    }


    /*
     * Display job information.
     */
    printf("\n");

    printf(
        "[CronD] Process created successfully.\n"
    );

    printf(
        "[CronD] Job ID       : %d\n",
        job_id
    );

    printf(
        "[CronD] Child PID    : %d\n",
        (int)pid
    );

    printf(
        "[CronD] Process PGID : %d\n",
        (int)pid
    );

    printf(
        "[CronD] Command      : %s\n",
        command
    );

    printf(
        "[CronD] Process state: RUNNING\n"
    );

    printf("\n");
}


/*
 * Execute a command synchronously.
 *
 * Parent waits until the child finishes.
 */
int execute_job(const char *command)
{
    pid_t pid = create_process();

    if (pid == -1)
    {
        return -1;
    }


    /*
     * Child process.
     */
    if (pid == 0)
    {
        setpgid(0, 0);

        execute_process(command);

        _exit(127);
    }


    /*
     * Parent process.
     */
    setpgid(pid, pid);

    int status;

    if (wait_for_process(
            pid,
            &status
        ) == -1)
    {
        return -1;
    }


    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    return -1;
}


/*
 * Execute an interactive foreground command.
 *
 * Example:
 *
 *     CronD> run cat > file.txt
 *
 * The child receives the controlling terminal.
 *
 * User can type:
 *
 *     Hello CronD
 *     This is my file.
 *
 * Then press:
 *
 *     Ctrl+D
 *
 * Ctrl+D sends EOF to cat.
 *
 * cat exits.
 *
 * Terminal control is returned to CronD.
 *
 * Then:
 *
 *     CronD>
 */
int execute_interactive(const char *command)
{
    /*
     * Create child process.
     */
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("[CronD] fork");

        return -1;
    }


    /*
     * ========================================
     * CHILD PROCESS
     * ========================================
     */
    if (pid == 0)
    {
        /*
         * Put child into its own process group.
         *
         * This process group will receive
         * terminal-generated signals.
         */
        if (setpgid(0, 0) == -1)
        {
            perror("[CronD] setpgid");

            _exit(1);
        }


        /*
         * Do not inherit CronD's signal handlers.
         *
         * The child should behave like a
         * normal Linux process.
         */
        reset_child_signal_handlers();


        /*
         * Execute command through /bin/sh.
         *
         * This allows shell features such as:
         *
         *     cat > file.txt
         *
         *     echo hello > file.txt
         *
         *     cat < file.txt
         */
        execl(
            "/bin/sh",
            "sh",
            "-c",
            command,
            (char *)NULL
        );


        /*
         * exec failed.
         */
        perror("[CronD] exec");

        _exit(127);
    }


    /*
     * ========================================
     * PARENT PROCESS
     * ========================================
     *
     * Parent also attempts to place the child
     * into its own process group.
     *
     * The child may already have done this,
     * so failure here is not automatically fatal.
     */
    if (setpgid(pid, pid) == -1)
    {
        /*
         * Check whether the process still exists.
         */
        if (getpgid(pid) == -1)
        {
            perror("[CronD] setpgid");

            return -1;
        }
    }


    /*
     * Give the controlling terminal to the
     * interactive child.
     *
     * This is the important job-control step.
     *
     * Before:
     *
     *     Terminal -> CronD
     *
     * After:
     *
     *     Terminal -> Child process group
     */
    if (give_terminal_to_process(pid) == -1)
    {
        fprintf(
            stderr,
            "[CronD] Could not give terminal to interactive process.\n"
        );


        /*
         * Terminate the child process group.
         */
        terminate_process_group(pid);


        /*
         * Wait for child cleanup.
         */
        waitpid(
            pid,
            NULL,
            0
        );


        /*
         * Restore terminal control.
         */
        restore_terminal_to_crond();

        return -1;
    }


    int status;


    /*
     * Wait for the interactive command.
     *
     * WUNTRACED allows CronD to detect
     * Ctrl+Z / stopped processes.
     */
    if (waitpid(
            pid,
            &status,
            WUNTRACED
        ) == -1)
    {
        perror("[CronD] waitpid");


        /*
         * Always restore terminal control.
         */
        restore_terminal_to_crond();

        return -1;
    }


    /*
     * IMPORTANT:
     *
     * Return terminal control to CronD
     * before showing the next prompt.
     */
    restore_terminal_to_crond();


    /*
     * Child was stopped.
     *
     * Usually caused by Ctrl+Z.
     */
    if (WIFSTOPPED(status))
    {
        printf(
            "[CronD] Interactive process stopped.\n"
        );

        return 0;
    }


    /*
     * Child was terminated by a signal.
     *
     * For example Ctrl+C -> SIGINT.
     */
    if (WIFSIGNALED(status))
    {
        printf(
            "[CronD] Interactive process terminated by signal %d.\n",
            WTERMSIG(status)
        );

        return 0;
    }


    /*
     * Child exited normally.
     */
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }


    return -1;
}
