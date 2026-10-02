#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "system_info.h"

void show_system_info(void)
{
    struct utsname system;

    if (uname(&system) == -1)
    {
        perror("uname");
        return;
    }

    printf("\n");
    printf("========================================\n");
    printf("          CronD System Information\n");
    printf("========================================\n");

    printf("Operating System : %s\n", system.sysname);
    printf("Kernel Release   : %s\n", system.release);
    printf("Kernel Version   : %s\n", system.version);
    printf("Architecture     : %s\n", system.machine);
    printf("CronD Process ID  : %d\n", getpid());

    printf("\nSystem Call Demonstration:\n");
    printf("  uname()  -> Kernel/System information\n");
    printf("  getpid() -> Current process ID\n");
    printf("  write()  -> Output through file descriptor\n");

    printf("\n");
}
