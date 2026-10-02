#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "jobs.h"
#include "service.h"

/*
 * 1 = CronD service is running
 * 0 = CronD service is stopped
 *
 * The CronD program itself remains alive until
 * the user enters the "exit" command.
 */
static int service_running = 1;

void start_service(void)
{
    pid_t pid = getpid();

    if (service_running)
    {
        printf("\n");
        printf("[CronD] Service is already running.\n");
        printf("[CronD] CronD PID: %d\n", (int)pid);
        printf("\n");
        return;
    }

    service_running = 1;

    printf("\n");
    printf("[CronD] Starting CronD service...\n");
    printf("[CronD] Running in user space.\n");
    printf("[CronD] CronD PID: %d\n", (int)pid);
    printf("[CronD] Service started successfully.\n");
    printf("\n");
}

void stop_service(void)
{
    if (!service_running)
    {
        printf("\n");
        printf("[CronD] Service is already stopped.\n");
        printf("\n");
        return;
    }

    service_running = 0;

    printf("\n");
    printf("[CronD] Stop request received.\n");
    printf("[CronD] Service stopped.\n");
    printf("\n");
}

void show_service_status(void)
{
    pid_t pid = getpid();

    printf("\n");
    printf("========================================\n");
    printf("          CronD Service Status\n");
    printf("========================================\n");

    if (service_running)
    {
        printf("Status       : RUNNING\n");
    }
    else
    {
        printf("Status       : STOPPED\n");
    }

    printf("CronD PID    : %d\n", (int)pid);
    printf("Running Jobs : %d\n", jobs_running_count());
    printf("Mode         : User Space\n");
    printf("\n");
}