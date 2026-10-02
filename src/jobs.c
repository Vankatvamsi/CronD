#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "jobs.h"

static job_t job_table[MAX_JOBS];

void jobs_init(void)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        job_table[i].job_id = 0;
        job_table[i].pid = 0;
        job_table[i].command[0] = '\0';
        job_table[i].state = JOB_EMPTY;
    }
}

int job_add(pid_t pid, const char *command)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].state == JOB_EMPTY)
        {
            job_table[i].job_id = i + 1;
            job_table[i].pid = pid;

            strncpy(
                job_table[i].command,
                command,
                MAX_COMMAND_LENGTH - 1
            );

            job_table[i].command[MAX_COMMAND_LENGTH - 1] = '\0';
            job_table[i].state = JOB_RUNNING;

            return job_table[i].job_id;
        }
    }

    return -1;
}

void jobs_update(void)
{
    int status;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].state == JOB_RUNNING)
        {
            pid_t result = waitpid(
                job_table[i].pid,
                &status,
                WNOHANG
            );

            if (result == job_table[i].pid)
            {
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    job_table[i].state = JOB_FINISHED;
                }
                else
                {
                    job_table[i].state = JOB_TERMINATED;
                }
            }
        }
    }
}

job_t *job_find(int job_id)
{
    if (job_id < 1 || job_id > MAX_JOBS)
    {
        return NULL;
    }

    if (job_table[job_id - 1].state == JOB_EMPTY)
    {
        return NULL;
    }

    return &job_table[job_id - 1];
}

void jobs_list(void)
{
    jobs_update();

    printf("\n");
    printf("============================================================\n");
    printf("                     CronD Job Table\n");
    printf("============================================================\n");
    printf("%-5s %-8s %-14s %s\n",
           "ID",
           "PID",
           "STATE",
           "COMMAND");
    printf("------------------------------------------------------------\n");

    int found = 0;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].state != JOB_EMPTY)
        {
            const char *state;

            switch (job_table[i].state)
            {
                case JOB_RUNNING:
                    state = "RUNNING";
                    break;

                case JOB_FINISHED:
                    state = "FINISHED";
                    break;

                case JOB_TERMINATED:
                    state = "TERMINATED";
                    break;

                default:
                    state = "UNKNOWN";
                    break;
            }

            printf("%-5d %-8d %-14s %s\n",
                   job_table[i].job_id,
                   (int)job_table[i].pid,
                   state,
                   job_table[i].command);

            found = 1;
        }
    }

    if (!found)
    {
        printf("No jobs available.\n");
    }

    printf("\n");
}

int job_remove(int job_id)
{
    job_t *job = job_find(job_id);

    if (job == NULL)
    {
        return -1;
    }

    job->job_id = 0;
    job->pid = 0;
    job->command[0] = '\0';
    job->state = JOB_EMPTY;

    return 0;
}

int jobs_running_count(void)
{
    jobs_update();

    int count = 0;

    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_table[i].state == JOB_RUNNING)
        {
            count++;
        }
    }

    return count;
}
