#ifndef WEB_SERVER_SIG_HANDLER_H
#define WEB_SERVER_SIG_HANDLER_H

static void worker_exit_handler_job(int signal);
static void all_workers_killed_job(int signal);

#endif //WEB_SERVER_SIG_HANDLER_H
