#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include "ipc.h"

static pid_t listener_pid = -1;
static int fifo_created = 0;

static void ipc_listener(void)
{
    char buffer[256];

    int fd = open(CROND_FIFO, O_RDONLY);

    if (fd == -1)
    {
        perror("[IPC] open");
        _exit(1);
    }

    printf("[IPC] FIFO listener started.\n");
    fflush(stdout);

    while (1)
    {
        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);

        if (bytes > 0)
        {
            buffer[bytes] = '\0';

            printf("[IPC] Message received: %s", buffer);
            fflush(stdout);
        }
        else if (bytes == 0)
        {
            close(fd);

            fd = open(CROND_FIFO, O_RDONLY);

            if (fd == -1)
            {
                _exit(1);
            }
        }
        else if (errno != EINTR)
        {
            perror("[IPC] read");
            break;
        }
    }

    close(fd);
    _exit(0);
}

int ipc_init(void)
{
    if (access(CROND_FIFO, F_OK) == 0)
    {
        fifo_created = 1;
        return 0;
    }

    if (mkfifo(CROND_FIFO, 0666) == -1)
    {
        perror("[IPC] mkfifo");
        return -1;
    }

    fifo_created = 1;

    return 0;
}

int ipc_start_listener(void)
{
    if (!fifo_created)
    {
        if (ipc_init() == -1)
        {
            return -1;
        }
    }

    if (listener_pid > 0)
    {
        printf("[IPC] Listener already running. PID: %d\n",
               (int)listener_pid);
        return 0;
    }

    listener_pid = fork();

    if (listener_pid == -1)
    {
        perror("[IPC] fork");
        return -1;
    }

    if (listener_pid == 0)
    {
        ipc_listener();
    }

    printf("[IPC] Listener process started.\n");
    printf("[IPC] Listener PID: %d\n", (int)listener_pid);

    return 0;
}

int ipc_send_message(const char *message)
{
    if (!fifo_created)
    {
        printf("[IPC] FIFO is not initialized.\n");
        return -1;
    }

    int fd = open(CROND_FIFO, O_WRONLY);

    if (fd == -1)
    {
        perror("[IPC] open for writing");
        return -1;
    }

    size_t length = strlen(message);

    if (write(fd, message, length) == -1)
    {
        perror("[IPC] write");
        close(fd);
        return -1;
    }

    if (write(fd, "\n", 1) == -1)
    {
        perror("[IPC] write");
        close(fd);
        return -1;
    }

    close(fd);

    printf("[IPC] Message sent successfully.\n");

    return 0;
}

void ipc_show_status(void)
{
    printf("\n");
    printf("========================================\n");
    printf("             CronD IPC Status\n");
    printf("========================================\n");

    if (fifo_created)
    {
        printf("FIFO Path     : %s\n", CROND_FIFO);
        printf("FIFO Status   : CREATED\n");
    }
    else
    {
        printf("FIFO Path     : %s\n", CROND_FIFO);
        printf("FIFO Status   : NOT CREATED\n");
    }

    if (listener_pid > 0)
    {
        printf("Listener PID  : %d\n", (int)listener_pid);
        printf("Listener      : RUNNING\n");
    }
    else
    {
        printf("Listener PID  : -\n");
        printf("Listener      : STOPPED\n");
    }

    printf("\n");
}

pid_t ipc_get_listener_pid(void)
{
    return listener_pid;
}

void ipc_cleanup(void)
{
    if (listener_pid > 0)
    {
        kill(listener_pid, SIGTERM);
        listener_pid = -1;
    }

    if (fifo_created)
    {
        unlink(CROND_FIFO);
        fifo_created = 0;
    }
}
