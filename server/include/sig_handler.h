#ifndef WEB_SERVER_SIG_HANDLER_H
#define WEB_SERVER_SIG_HANDLER_H

void init_sig_handler(int sd);
void worker_exit_handler_job(int signal);
void all_workers_killed_job(int signal);
void add_worker(void);

#endif //WEB_SERVER_SIG_HANDLER_H
