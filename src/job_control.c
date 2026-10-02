#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include "job_control.h"

int create_process_group(pid_t pid)
{
    if (setpgid(pid, pid) == -1)
    {
        perror("[JobControl] setpgid");
        return -1;
    }

    return 0;
}

int terminate_process_group(pid_t pgid)
{
    if (kill(-pgid, SIGTERM) == -1)
    {
        perror("[JobControl] kill process group");
        return -1;
    }

    return 0;
}

int get_process_group_info(pid_t pid)
{
    pid_t pgid = getpgid(pid);

    if (pgid == -1)
    {
        perror("[JobControl] getpgid");
        return -1;
    }

    printf("[JobControl] PID  : %d\n", (int)pid);
    printf("[JobControl] PGID : %d\n", (int)pgid);

    return 0;
}
