#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "process.h"

pid_t create_process(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    return pid;
}

int execute_process(const char *command)
{
    execl(
        "/bin/sh",
        "sh",
        "-c",
        command,
        (char *)NULL
    );

    perror("exec");
    return -1;
}

int terminate_process(pid_t pid)
{
    if (kill(pid, SIGTERM) == -1)
    {
        perror("kill");
        return -1;
    }

    return 0;
}

int wait_for_process(pid_t pid, int *status)
{
    pid_t result = waitpid(pid, status, 0);

    if (result == -1)
    {
        perror("waitpid");
        return -1;
    }

    return 0;
}
