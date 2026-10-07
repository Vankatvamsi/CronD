#ifndef EXECUTOR_H
#define EXECUTOR_H

void execute_job_async(const char *command);

int execute_job(const char *command);

/*
 * Execute an interactive command in foreground.
 * Used for commands such as:
 *
 *     cat > file.txt
 *
 * The CronD prompt waits until the command finishes.
 */
int execute_interactive(const char *command);

#endif
