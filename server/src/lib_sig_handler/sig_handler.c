//
// Created by ns on 27.09.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <logger.h>

int worker_count = 0;

// отправляет текущему процессу SIGUSR1, если все дети завершились
void worker_exit_handler_job(int signal) {
    pid_t pid;
    int status;
    while ((pid = waitpid(WAIT_ANY, &status, WNOHANG)) > 0) {
        const int msg_max_size = 50;
        char str[msg_max_size];
        str[0] = '\0';
        if (WIFEXITED(status)) {
            sprintf(str, "Worker %d exited with code %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            sprintf(str, "Worker %d killed by signal %d\n", pid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            sprintf(str, "Worker %d stopped by signal %d\n", pid, WSTOPSIG(status));
        }
        info_log(str);
        worker_count--;
    }
    // если убит последний воркер -- нужно залогировать и завершить работу процесса, для этого вызываем
    if (worker_count == 0) {
        kill(getpid(), SIGUSR1);
    }
    // что если произошел exit() воркера? нужно пересоздавать?
}

void all_workers_killed_job(int signal) {
    info_log("All workers stopped or terminated");
    sleep(5);
    exit(0);
}

void add_worker(void) {
    worker_count++;
}

int get_worker_count(void) {
    return worker_count;
}
