#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 50
#define MAX_COMMAND_LENGTH 256

typedef enum
{
    JOB_EMPTY,
    JOB_RUNNING,
    JOB_FINISHED,
    JOB_TERMINATED
} job_state_t;

typedef struct
{
    int job_id;
    pid_t pid;
    char command[MAX_COMMAND_LENGTH];
    job_state_t state;
} job_t;

void jobs_init(void);

int job_add(pid_t pid, const char *command);

void jobs_update(void);

void jobs_list(void);

job_t *job_find(int job_id);

int job_remove(int job_id);

int jobs_running_count(void);

#endif
