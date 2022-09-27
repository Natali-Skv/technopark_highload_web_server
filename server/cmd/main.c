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
#include <sig_handler.h>
#include <logger.h>

int cpu_count = 4;


int start_server() {
//     добавить обработку сигнала сигчайнд
    init_logger();
    signal(SIGCHLD, worker_exit_handler_job);
    signal(SIGUSR1, all_workers_killed_job);
    for (; cpu_count > 0; --cpu_count) {
        int pid = fork();
        if (pid == -1) {
            int err = errno;
            fprintf(stderr, "Error forking: %s\n", strerror(err));
            return err;
        }

        if (pid == 0) {
            printf("CHILD: Hello world\n");
            // вешаем всю работу на этот процесс
            exit(0);
        }
    }
    sleep(5);
}

// форкнуть в цикле
// добавить в массив пиды?? нужно ли иметь массив пидов процессов?
// ? нужно удалять зомби ? они вообще появятся? вроде нет
// повесить обработку сигналов
// нужен 1 родительский процесс, а воркеров -- кол-во из конфига -1 ?
// если процесс будет делать exit() надо писать чистку зомби/ обработку сигнала sigchild
int main(void) {
    // парсинг конфига
    // префорк

    // сделать обработку разных сигналовдля процессов
    // открыть сокет
    // сделать ивент луп   ? есть вопросы по порядку открытия сокета и объявления ивент-лупа

    // подписаться на сигчайлд


    start_server();

    return 0;
}
