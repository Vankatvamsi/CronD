#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "session.h"

pid_t get_current_session_id(void)
{
    return getsid(0);
}

pid_t get_current_process_group(void)
{
    return getpgrp();
}

void show_session_info(void)
{
    pid_t pid = getpid();
    pid_t ppid = getppid();
    pid_t pgid = getpgrp();
    pid_t sid = getsid(0);

    printf("\n");
    printf("========================================\n");
    printf("          CronD Session Information\n");
    printf("========================================\n");

    printf("Process ID        : %d\n", (int)pid);
    printf("Parent Process ID : %d\n", (int)ppid);
    printf("Process Group ID  : %d\n", (int)pgid);
    printf("Session ID        : %d\n", (int)sid);

    printf("\n");
}
