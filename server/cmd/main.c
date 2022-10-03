#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sig_handler.h>
#include <logger.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ev.h>
#include <limits.h>
#include <stdbool.h>
#include <getopt.h>
#include <http.h>
#include <limits.h>
#include <stdlib.h>

#define MAX_LEN_ROOT_PATH 100
#define DEFAULT_CPU_LIMIT 4
#define PORT 80
#define DEFAULT_DOCUMENT_ROOT "/var/www/html"


struct config_t {
    int port;
    int cpu_limit;
    char document_root[MAX_LEN_ROOT_PATH];
};

int set_socket(int port, int *sock_fd) {
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        int err_code = errno;
        err_log_code("error creating socket", err_code);
        return err_code;
    }

    int param_on = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &param_on, sizeof(int)) == -1) {
        close(listener);
        int err_code = errno;
        err_log_code("error setting options on socket", err_code);
        return err_code;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        int err_code = errno;
        err_log_code("error binding socket", err_code);
        return err_code;
    }

    if (listen(listener, INT_MAX) < 0) {
        int err_code = errno;
        err_log_code("error listening socket", err_code);
        return err_code;
    }
    *sock_fd = listener;
    return 0;
}

static void read_cb(struct ev_loop *loop, ev_io *watcher, int revents)
{
    printf("---PROCESSING REQUEST----");
    get_http_response_cb(watcher->fd);
    printf("---END PROCESSING REQUEST----");
    printf(" %d\n",__LINE__);
//    http_cb(watcher->fd, "/home/ns/tp/hl/tests_for_web_server/" );
    close(watcher->fd);
    printf(" %d\n",__LINE__);
    ev_io_stop(EV_A_ watcher);
    printf(" %d\n",__LINE__);
    free(watcher);
    printf(" %d\n",__LINE__);
}

static void accept_cb(EV_P_ ev_io *watcher, int revents)
{
    printf(" %d\n",__LINE__);
    int connfd;
    ev_io *client;
    printf(" %d\n",__LINE__);
    connfd = accept(watcher->fd, NULL, NULL);

    printf(" %d\n",__LINE__);
    if (connfd > 0) {
        printf(" %d\n",__LINE__);
        printf(" %d\n",__LINE__);
        client = calloc(1, sizeof(*client));
        printf(" %d\n",__LINE__);
        ev_io_init(client, read_cb, connfd, EV_READ);
        printf(" %d\n",__LINE__);
        ev_io_start(EV_A_ client);

        printf(" %d\n",__LINE__);
    } else if ((connfd < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        printf(" %d\n",__LINE__);
        return;

    } else {
        printf(" %d\n",__LINE__);
        fprintf(stdout, "errno %d\n", errno);
        printf(" %d\n",__LINE__);
        close(watcher->fd);
        printf(" %d\n",__LINE__);
        ev_break(EV_A_ EVBREAK_ALL);
        /* this will lead main to exit, no need to free watchers of clients */
    }
    printf(" %d\n",__LINE__);
}

//static void accept_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
//    printf(" %d\n",__LINE__);
//    int conn_fd = accept(watcher->fd, NULL, NULL);
//
//    printf(" %d\n",__LINE__);
//    if (conn_fd > 0) {
//        printf(" %d\n",__LINE__);
//        // TODO: возможно стоит раскоментить -- установить таймаут на отправку данных для текущего клиента
////        struct timeval timeout;
////        timeout.tv_sec = 60;
////        timeout.tv_usec = 0;
////        if (setsockopt(conn_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) == -1) {
////            close(conn_fd);
////            int err_code = errno;
////            err_log_code("error setting options on socket", err_code);
////            return;
////        }
//        ev_io *client_watcher = calloc(1, sizeof(*client_watcher));
//        printf(" %d\n",__LINE__);
//        ev_io_init(client_watcher, read_cb, conn_fd, EV_READ);
//        printf(" %d\n",__LINE__);
//        ev_io_start(EV_A_ client_watcher);
//        printf(" %d\n",__LINE__);
//    } else if ((conn_fd < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
//        printf(" %d\n",__LINE__);
//        return;
//    } else {
//        printf(" %d\n",__LINE__);
//        err_log_code("error accepting connection", errno);
//    }
//}

int run_server(int cpu_limit, int sock_fd) {
    signal(SIGCHLD, worker_exit_handler_job);
    signal(SIGUSR1, all_workers_killed_job);
    signal(SIGPIPE, SIG_IGN);
    struct ev_loop *loop = ev_default_loop(EVBACKEND_EPOLL);
    if (loop == NULL) {
        int err_code = errno;
        err_log_code("error creating default event loop", err_code);
        return err_code;
    }
    ev_io *watcher = NULL;
    int num_process_to_fork = cpu_limit;
    for (; cpu_limit > 0; --cpu_limit) {
        int pid = fork();
        if (pid == -1) {
            int err_code = errno;
            err_log_code("error forking", err_code);
            return err_code;
        }

        if (pid == 0) {
            add_worker();
            watcher = calloc(1, sizeof(*watcher));
            if (watcher == NULL) {
                int err_code = errno;
                err_log_code("error creating watcher", err_code);
                return err_code;
            }
            ev_io_init(watcher, accept_cb, sock_fd, EV_READ);
            ev_io_start(loop, watcher);
            ev_loop_fork(loop);
            ev_run(loop, 0);
            num_process_to_fork--;
            ev_loop_destroy(loop);
            free(watcher);
            exit(0);
        }
    }

    ev_loop_destroy(loop);
    free(watcher);

    while (true) {
        sleep(INT_MAX);
    }
    return 0;
}

// раздел \r\n переводом lib_http-запросов/ ответов
// сокет клиента может закрыться до того или во время того, как сервер будет отправлять ответ
// сделать shut_down socket завершением процесса
// добавить в массив пиды?? нужно ли иметь массив пидов процессов?
// ? нужно удалять зомби ? они вообще появятся? вроде нет
// нужен 1 родительский процесс, а воркеров -- кол-во из конфига -1 ?

int set_config(int argc, char *argv[], struct config_t *cfg) {

    struct option options[] = {
            {"help",          no_argument,       NULL, 'h'},
            {"cpu_limit",     required_argument, NULL, 'c'},
            {"document_root", required_argument, NULL, 'r'},
            {NULL, 0,                            NULL, 0}
    };
    int c, opt_idx = 0;
    while ((c = getopt_long(argc, argv, "-c:r:", options, &opt_idx)) != -1) {
        switch (c) {
            case 0: {
                printf("long option %s", options[opt_idx].name);
                if (optarg) { printf(" with arg %s", optarg); }
                return -1;
            }
            case 1: {
                printf("non-option argument %s\n", optarg);
                return -1;
            }
            case 'c': {
                cfg->cpu_limit = atoi(optarg);
                if (cfg->cpu_limit <= 0) {
                    printf("cpu limit must be positive number\n");
                    return -1;
                }
                break;
            }
            case 'r': {
                if (optarg == NULL) {
                    printf("root document path must have [1..50] symbols\n");
                    return -1;
                }
                unsigned long optarg_len = strlen(optarg);
                if (optarg_len > MAX_LEN_ROOT_PATH || optarg_len <= 0) {
                    printf("root document path must have [1..50] symbols\n");
                    return -1;
                }
                strncpy(cfg->document_root, optarg, sizeof(cfg->document_root));
                break;
            }
            case 'h': {
                printf("Usage: %s [--help|--document_root <path>|--cpu_limit <num>] [-r <path>|-c <num>]\n", argv[0]);
                return -1;
            }
            case '?': {
                printf("Unknown option %c\n", optopt);
                break;
            }
            case ':': {
                printf("Missing argument for %c\n", optopt);
                break;
            }
            default:
                break;
        }
    }

    printf("%d, %s\n", cfg->cpu_limit, cfg->document_root);
    if (optind < argc) {
        printf("unknown argument: %s\n", argv[optind]);
        return -1;
    }
    return 0;
}

void set_config_default_values(struct config_t *cfg) {
    if (cfg->cpu_limit == 0) {
        cfg->cpu_limit = DEFAULT_CPU_LIMIT;
    }
    if (strcmp(cfg->document_root, "") == 0) {
        strcpy(cfg->document_root, DEFAULT_DOCUMENT_ROOT);
    }
    if (cfg->port == 0) {
        cfg->port = PORT;
    }
}

int main(int argc, char *argv[]) {
    init_logger();

    struct config_t cfg = {0};
    if (set_config(argc, argv, &cfg) != 0) {
        return 0;
    }

    set_config_default_values(&cfg);
    init_document_root(cfg.document_root);

    int sock_fd = -1;
    if (set_socket(cfg.port, &sock_fd) != 0) {
        return 0;
    }

    if (run_server(cfg.cpu_limit, sock_fd) != 0) {
        return 0;
    }

    return 0;
}
