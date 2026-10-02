#ifndef SIGNALS_H
#define SIGNALS_H

void signals_init(void);

void handle_sigchld(int signal);
void handle_sigterm(int signal);
void handle_sighup(int signal);
void handle_sigint(int signal);

#endif
