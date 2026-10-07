#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cli.h"
#include "service.h"
#include "system_info.h"
#include "jobs.h"
#include "executor.h"
#include "process.h"
#include "ipc.h"
#include "signals.h"
#include "job_control.h"
#include "session.h"
#include "history.h"


/*
 * CronD startup banner
 */
void show_banner(void)
{
    printf("\n");
    printf("========================================\n");
    printf("        CronD - Job Scheduler Daemon\n");
    printf("========================================\n");
    printf("[CronD] Service started successfully.\n");
    printf("[CronD] Ready to accept commands.\n");
    printf("Type 'help' to see available commands.\n");
    printf("Type 'exit' to leave CronD.\n");
    printf("\n");
}


/*
 * Display available commands.
 */
void show_help(void)
{
    printf("\n");

    printf("Process Commands:\n");
    printf("-----------------\n");
    printf("  run <command>     Create and execute a process\n");
    printf("  jobs              Show all jobs\n");
    printf("  kill <job_id>     Terminate a running job\n");
    printf("\n");

    printf("IPC Commands:\n");
    printf("-------------\n");
    printf("  ipc-start         Start FIFO IPC listener\n");
    printf("  ipc-send <msg>    Send message through FIFO\n");
    printf("  ipc-status        Show IPC status\n");
    printf("\n");

    printf("Process Group / Session:\n");
    printf("------------------------\n");
    printf("  groups            Show CronD process group\n");
    printf("  session           Show CronD session information\n");
    printf("\n");

    printf("Service Commands:\n");
    printf("-----------------\n");
    printf("  start             Start CronD service\n");
    printf("  stop              Stop CronD service\n");
    printf("  status            Show CronD service status\n");
    printf("\n");

    printf("System Commands:\n");
    printf("----------------\n");
    printf("  info              Show Linux system information\n");
    printf("  help              Show available commands\n");
    printf("  exit              Exit CronD\n");
    printf("\n");

    printf("Command History:\n");
    printf("----------------\n");
    printf("  Up Arrow          Previous command\n");
    printf("  Down Arrow        Next command\n");
    printf("\n");
}


/*
 * Main CronD command-line interface.
 */
void run_cli(void)
{
    char command[512];

    /*
     * Initialize job management.
     */
    jobs_init();

    /*
     * Register signal handlers.
     */
    signals_init();

    /*
     * Display CronD banner.
     */
    show_banner();

    while (1)
    {
        printf("CronD> ");
        fflush(stdout);

        /*
         * Read command using the CronD
         * command-history input system.
         *
         * Up Arrow:
         *     Previous command
         *
         * Down Arrow:
         *     Next command
         *
         * Enter:
         *     Execute command
         */
        if (history_read_command(
                command,
                sizeof(command)
            ) == -1)
        {
            printf(
                "\n[CronD] Input closed. Exiting...\n"
            );

            ipc_cleanup();

            break;
        }


        /*
         * Ignore empty commands.
         */
        if (strlen(command) == 0)
        {
            continue;
        }


        /*
         * ========================================
         * HELP
         * ========================================
         */
        if (strcmp(command, "help") == 0)
        {
            show_help();
        }


        /*
         * ========================================
         * SERVICE COMMANDS
         * ========================================
         */

        else if (strcmp(command, "start") == 0)
        {
            start_service();
        }

        else if (strcmp(command, "stop") == 0)
        {
            stop_service();
        }

        else if (strcmp(command, "status") == 0)
        {
            show_service_status();
        }


        /*
         * ========================================
         * SYSTEM INFORMATION
         * ========================================
         */

        else if (strcmp(command, "info") == 0)
        {
            show_system_info();
        }


        /*
         * ========================================
         * JOB MANAGEMENT
         * ========================================
         */

        else if (strcmp(command, "jobs") == 0)
        {
            jobs_list();
        }


        /*
         * ========================================
         * SESSION INFORMATION
         * ========================================
         */

        else if (strcmp(command, "session") == 0)
        {
            show_session_info();
        }


        /*
         * ========================================
         * PROCESS GROUP INFORMATION
         * ========================================
         */

        else if (strcmp(command, "groups") == 0)
        {
            printf("\n");

            get_process_group_info(getpid());

            printf("\n");
        }


        /*
         * ========================================
         * START IPC LISTENER
         * ========================================
         */

        else if (strcmp(command, "ipc-start") == 0)
        {
            if (ipc_init() == 0)
            {
                ipc_start_listener();
            }
        }


        /*
         * ========================================
         * IPC STATUS
         * ========================================
         */

        else if (strcmp(command, "ipc-status") == 0)
        {
            ipc_show_status();
        }


        /*
         * ========================================
         * SEND IPC MESSAGE
         * ========================================
         */

        else if (strncmp(
                    command,
                    "ipc-send ",
                    9
                ) == 0)
        {
            const char *message =
                command + 9;

            if (strlen(message) == 0)
            {
                printf(
                    "[CronD] Usage: ipc-send <message>\n"
                );
            }
            else
            {
                ipc_send_message(message);
            }
        }


        /*
         * ========================================
         * RUN COMMAND
         * ========================================
         *
         * Normal commands:
         *
         *     run ls
         *     run pwd
         *     run date
         *     run sleep 20
         *
         * are executed asynchronously.
         *
         * Interactive commands:
         *
         *     run cat > file.txt
         *
         * are executed in foreground mode.
         */
        else if (strncmp(
                    command,
                    "run ",
                    4
                ) == 0)
        {
            const char *job_command =
                command + 4;

            if (strlen(job_command) == 0)
            {
                printf(
                    "[CronD] Usage: run <command>\n"
                );
            }


            /*
             * Interactive file creation.
             *
             * Example:
             *
             *     run cat > file1.txt
             */
            else if (
                strncmp(
                    job_command,
                    "cat >",
                    5
                ) == 0 ||

                strncmp(
                    job_command,
                    "cat> ",
                    5
                ) == 0
            )
            {
                execute_interactive(
                    job_command
                );
            }


            /*
             * Normal asynchronous command.
             */
            else
            {
                execute_job_async(
                    job_command
                );
            }
        }


        /*
         * ========================================
         * KILL JOB
         * ========================================
         */

        else if (strncmp(
                    command,
                    "kill ",
                    5
                ) == 0)
        {
            int job_id =
                atoi(command + 5);

            job_t *job =
                job_find(job_id);

            if (job == NULL)
            {
                printf(
                    "[CronD] Job %d not found.\n",
                    job_id
                );
            }

            else if (
                job->state != JOB_RUNNING
            )
            {
                printf(
                    "[CronD] Job %d is not running.\n",
                    job_id
                );
            }

            else
            {
                if (terminate_process(
                        job->pid
                    ) == 0)
                {
                    printf(
                        "[CronD] Termination signal sent to Job %d.\n",
                        job_id
                    );
                }
            }
        }


        /*
         * ========================================
         * EXIT
         * ========================================
         */

        else if (
            strcmp(
                command,
                "exit"
            ) == 0
        )
        {
            printf("\n");

            printf(
                "[CronD] Cleaning up IPC resources...\n"
            );

            ipc_cleanup();

            printf(
                "[CronD] Exiting CronD...\n"
            );

            break;
        }


        /*
         * ========================================
         * UNKNOWN COMMAND
         * ========================================
         */

        else
        {
            printf(
                "[CronD] Unknown command: %s\n",
                command
            );

            printf(
                "Type 'help' to see available commands.\n"
            );
        }
    }
}
