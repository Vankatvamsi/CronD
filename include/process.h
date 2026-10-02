#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

pid_t create_process(void);

int execute_process(const char *command);

int terminate_process(pid_t pid);

int wait_for_process(pid_t pid, int *status);

#endif
