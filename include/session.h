#ifndef SESSION_H
#define SESSION_H

#include <sys/types.h>

void show_session_info(void);

pid_t get_current_session_id(void);

pid_t get_current_process_group(void);

#endif
