#include <logger.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sig_handler.h>

int worker_count = 0;
int sock_fd = 0;

void init_sig_handler(int sd) {
    sock_fd = sock_fd;
}

// отправляет текущему процессу SIGUSR1, если все воркеры завершились
void worker_exit_handler_job(int signal) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            info_log("Worker %d exited with code %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            info_log("Worker %d killed by signal %d\n", pid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            info_log("Worker %d stopped by signal %d\n", pid, WSTOPSIG(status));
        }
        worker_count--;
        printf("   %d", worker_count);
    }
    if (worker_count == 0) {
        all_workers_killed_job(SIGINT);
    }
}

void all_workers_killed_job(int signal) {
    info_log("All workers stopped or terminated");
    destruct_logger();
    close(sock_fd);
    exit(0);
}

void add_worker(void) {
    worker_count++;
}
