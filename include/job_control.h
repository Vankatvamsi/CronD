#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>

int create_process_group(pid_t pid);

int terminate_process_group(pid_t pgid);

int get_process_group_info(pid_t pid);

/*
 * Give the controlling terminal to a process group.
 */
int give_terminal_to_process(pid_t pgid);

/*
 * Return the controlling terminal to CronD.
 */
int restore_terminal_to_crond(void);

#endif
