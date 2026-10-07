#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "history.h"

#define HISTORY_SIZE 50

/*
 * Store previously entered CronD commands.
 */
static char command_history[HISTORY_SIZE][512];

static int history_count = 0;


/*
 * Original terminal configuration.
 */
static struct termios original_terminal;

static int terminal_raw = 0;


/*
 * Restore normal terminal mode.
 */
static void restore_terminal(void)
{
    if (terminal_raw)
    {
        tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &original_terminal
        );

        terminal_raw = 0;
    }
}


/*
 * Enable character-by-character input.
 */
static int enable_raw_terminal(void)
{
    struct termios raw;

    if (!isatty(STDIN_FILENO))
    {
        return -1;
    }

    if (tcgetattr(
            STDIN_FILENO,
            &original_terminal
        ) == -1)
    {
        return -1;
    }

    raw = original_terminal;

    /*
     * Disable canonical mode and echo.
     */
    raw.c_lflag &= (tcflag_t)~(
        ICANON | ECHO
    );

    /*
     * Read one character at a time.
     */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &raw
        ) == -1)
    {
        return -1;
    }

    terminal_raw = 1;

    return 0;
}


/*
 * Add a command to history.
 */
static void history_add(const char *command)
{
    if (command == NULL ||
        command[0] == '\0')
    {
        return;
    }


    /*
     * Don't store duplicate consecutive commands.
     */
    if (history_count > 0 &&
        strcmp(
            command_history[history_count - 1],
            command
        ) == 0)
    {
        return;
    }


    /*
     * History has available space.
     */
    if (history_count < HISTORY_SIZE)
    {
        strncpy(
            command_history[history_count],
            command,
            sizeof(command_history[0]) - 1
        );

        command_history[history_count]
                      [sizeof(command_history[0]) - 1]
                      = '\0';

        history_count++;

        return;
    }


    /*
     * History is full.
     *
     * Remove the oldest command.
     */
    memmove(
        command_history,
        command_history + 1,
        (HISTORY_SIZE - 1) *
        sizeof(command_history[0])
    );


    /*
     * Add the newest command.
     */
    strncpy(
        command_history[HISTORY_SIZE - 1],
        command,
        sizeof(command_history[0]) - 1
    );

    command_history[HISTORY_SIZE - 1]
                  [sizeof(command_history[0]) - 1]
                  = '\0';
}


/*
 * Redraw CronD command line.
 */
static void redraw_line(const char *buffer)
{
    /*
     * \r  -> move cursor to beginning
     * \033[K -> clear current line
     */
    printf(
        "\r\033[KCronD> %s",
        buffer
    );

    fflush(stdout);
}


/*
 * Read a command with Up/Down history.
 */
static int read_history_command(
    char *buffer,
    int size
)
{
    int length = 0;

    /*
     * Points after the newest history entry.
     */
    int history_index = history_count;

    int browsing_history = 0;

    buffer[0] = '\0';


    /*
     * Enable raw terminal mode.
     */
    if (enable_raw_terminal() == -1)
    {
        return -2;
    }


    while (1)
    {
        unsigned char c;

        ssize_t bytes_read = read(
            STDIN_FILENO,
            &c,
            1
        );


        /*
         * Handle read error.
         */
        if (bytes_read == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            restore_terminal();

            return -1;
        }


        /*
         * End of input.
         */
        if (bytes_read == 0)
        {
            restore_terminal();

            return -1;
        }


        /*
         * =====================================
         * ENTER
         * =====================================
         */
        if (c == '\n' ||
            c == '\r')
        {
            buffer[length] = '\0';

            putchar('\n');

            fflush(stdout);

            restore_terminal();

            /*
             * Save command in history.
             */
            history_add(buffer);

            return 0;
        }


        /*
         * =====================================
         * CTRL+D
         * =====================================
         */
        if (c == 4)
        {
            /*
             * Ctrl+D on an empty command means EOF.
             */
            if (length == 0)
            {
                putchar('\n');

                fflush(stdout);

                restore_terminal();

                return -1;
            }

            continue;
        }


        /*
         * =====================================
         * BACKSPACE
         * =====================================
         */
        if (c == 8 ||
            c == 127)
        {
            if (length > 0)
            {
                length--;

                buffer[length] = '\0';

                printf("\b \b");

                fflush(stdout);

                browsing_history = 0;

                history_index = history_count;
            }

            continue;
        }


        /*
         * =====================================
         * ESCAPE SEQUENCE
         * =====================================
         *
         * Up Arrow:
         *
         *     ESC [ A
         *
         * Down Arrow:
         *
         *     ESC [ B
         */
        if (c == 27)
        {
            unsigned char sequence_1;
            unsigned char sequence_2;


            /*
             * Read '['.
             */
            if (read(
                    STDIN_FILENO,
                    &sequence_1,
                    1
                ) != 1)
            {
                continue;
            }


            if (sequence_1 != '[')
            {
                continue;
            }


            /*
             * Read A or B.
             */
            if (read(
                    STDIN_FILENO,
                    &sequence_2,
                    1
                ) != 1)
            {
                continue;
            }


            /*
             * =================================
             * UP ARROW
             * =================================
             */
            if (sequence_2 == 'A')
            {
                if (history_count == 0)
                {
                    continue;
                }


                /*
                 * First Up Arrow:
                 * show most recent command.
                 */
                if (!browsing_history)
                {
                    history_index =
                        history_count - 1;

                    browsing_history = 1;
                }


                /*
                 * Further Up Arrow:
                 * move backwards.
                 */
                else if (history_index > 0)
                {
                    history_index--;
                }


                /*
                 * Copy selected command.
                 */
                strncpy(
                    buffer,
                    command_history[history_index],
                    size - 1
                );

                buffer[size - 1] = '\0';

                length =
                    (int)strlen(buffer);


                /*
                 * Display command.
                 */
                redraw_line(buffer);
            }


            /*
             * =================================
             * DOWN ARROW
             * =================================
             */
            else if (sequence_2 == 'B')
            {
                if (!browsing_history)
                {
                    continue;
                }


                /*
                 * Move toward newer commands.
                 */
                if (history_index <
                    history_count - 1)
                {
                    history_index++;

                    strncpy(
                        buffer,
                        command_history[history_index],
                        size - 1
                    );

                    buffer[size - 1] = '\0';

                    length =
                        (int)strlen(buffer);
                }


                /*
                 * Move past newest command.
                 *
                 * This gives a fresh empty prompt.
                 */
                else
                {
                    history_index =
                        history_count;

                    buffer[0] = '\0';

                    length = 0;

                    browsing_history = 0;
                }


                redraw_line(buffer);
            }

            continue;
        }


        /*
         * Ignore other control characters.
         */
        if (c < 32)
        {
            continue;
        }


        /*
         * =====================================
         * NORMAL CHARACTER
         * =====================================
         */
        if (length < size - 1)
        {
            buffer[length++] =
                (char)c;

            buffer[length] = '\0';

            putchar(c);

            fflush(stdout);


            /*
             * Once the user starts typing,
             * leave history browsing mode.
             */
            browsing_history = 0;

            history_index = history_count;
        }
    }
}


/*
 * Public history command reader.
 */
int history_read_command(
    char *buffer,
    int size
)
{
    if (buffer == NULL ||
        size <= 1)
    {
        return -1;
    }


    /*
     * Use arrow-key history when running
     * inside a real terminal.
     */
    if (isatty(STDIN_FILENO))
    {
        int result =
            read_history_command(
                buffer,
                size
            );

        /*
         * -2 means terminal mode could not
         * be enabled, so use fgets fallback.
         */
        if (result != -2)
        {
            return result;
        }
    }


    /*
     * Fallback for non-interactive input.
     */
    if (fgets(
            buffer,
            (size_t)size,
            stdin
        ) == NULL)
    {
        return -1;
    }


    buffer[strcspn(
        buffer,
        "\n"
    )] = '\0';


    history_add(buffer);

    return 0;
}
