#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define CROND_FIFO "/tmp/crond_fifo"

int ipc_init(void);

int ipc_start_listener(void);

int ipc_send_message(const char *message);

void ipc_show_status(void);

void ipc_cleanup(void);

pid_t ipc_get_listener_pid(void);

#endif
