#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "executor.h"
#include "process.h"
#include "jobs.h"
#include "job_control.h"

void execute_job_async(const char *command)
{
    pid_t pid = create_process();

    if (pid == -1)
    {
        printf("[CronD] Failed to create process.\n");
        return;
    }

    if (pid == 0)
    {
        /*
         * Child process creates its own process group.
         */
        if (setpgid(0, 0) == -1)
        {
            perror("[CronD] setpgid");
            _exit(1);
        }

        execute_process(command);

        _exit(127);
    }

    /*
     * Parent also attempts to place child into its own group.
     */
    setpgid(pid, pid);

    int job_id = job_add(pid, command);

    if (job_id == -1)
    {
        printf("[CronD] Job table is full.\n");
        terminate_process(pid);
        return;
    }

    printf("\n");
    printf("[CronD] Process created successfully.\n");
    printf("[CronD] Job ID      : %d\n", job_id);
    printf("[CronD] Child PID   : %d\n", (int)pid);
    printf("[CronD] Process PGID: %d\n", (int)pid);
    printf("[CronD] Command     : %s\n", command);
    printf("[CronD] Process state: RUNNING\n");
    printf("\n");
}

int execute_job(const char *command)
{
    pid_t pid = create_process();

    if (pid == -1)
    {
        return -1;
    }

    if (pid == 0)
    {
        setpgid(0, 0);

        execute_process(command);

        _exit(127);
    }

    setpgid(pid, pid);

    int status;

    if (wait_for_process(pid, &status) == -1)
    {
        return -1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    return -1;
}