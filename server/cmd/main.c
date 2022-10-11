#include <errno.h>
#include <ev.h>
#include <fcntl.h>
#include <http.h>
#include <limits.h>
#include <logger.h>
#include <netinet/in.h>
#include <server.h>
#include <set_config.h>
#include <sig_handler.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define CREATING_SOCKET_ERR (-1)
#define SET_SOCK_OPT_ERR (-2)
#define BINDING_SOCKET_ERR (-3)
#define LISTENING_SOCKET_ERR (-4)
#define CREATING_EVLOOP_ERR (-5)
#define FORKING_ERR (-6)

struct w_client_t {
    ev_io io;
    struct response_t *resp;
};

int set_socket(int port, int *sock_fd);
int run_server(int cpu_limit, int sock_fd);

int main(int argc, char *argv[]) {
    struct config_t cfg = {0};
    if (set_config(argc, argv, &cfg) != OK) {
        return 0;
    }
    set_config_default_values(&cfg);

    int init_logger_res = init_logger(cfg.server_log_path, cfg.access_log_path);
    if (init_logger_res == OPENING_SERVER_LOG_FILE_ERR) {
        printf("error opening server log file: %s", strerror(errno));
        return 0;
    }
    if (init_logger_res == OPENING_ACCESS_LOG_FILE_ERR) {
        printf("error opening access log file: %s", strerror(errno));
        return 0;
    }

    if (init_document_root(cfg.document_root) == UNRESOLVING_ROOT_PATH) {
        printf("error resolving root path: %s", strerror(errno));
        destruct_logger();
        return 0;
    }

    int sock_fd = -1;
    int set_socket_res = set_socket(cfg.port, &sock_fd);
    if (set_socket_res == CREATING_SOCKET_ERR) {
        printf("error creating socket: %s", strerror(errno));
        destruct_logger();
        return 0;
    } else if (set_socket_res == SET_SOCK_OPT_ERR) {
        printf("error setting socket options: %s", strerror(errno));
        destruct_logger();
        return 0;
    } else if (set_socket_res == BINDING_SOCKET_ERR) {
        printf("error bilding socket: %s", strerror(errno));
        destruct_logger();
        return 0;
    } else if (set_socket_res == LISTENING_SOCKET_ERR) {
        printf("error listening socket: %s", strerror(errno));
        destruct_logger();
        return 0;
    }

    init_sig_handler(sock_fd);

    info_log("starting server with cpu limit: %d, document root: %s\n", cfg.cpu_limit, cfg.document_root);
    if (run_server(cfg.cpu_limit, sock_fd) != 0) {
        destruct_logger();
        close(sock_fd);
        return 0;
    }

    return 0;
}

int set_socket(int port, int *sock_fd) {
    *sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (*sock_fd < 0) {
        return CREATING_SOCKET_ERR;
    }

    int param_on = 1;
    if (setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, &param_on, sizeof(int)) != 0) {
        close(*sock_fd);
        return SET_SOCK_OPT_ERR;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        return BINDING_SOCKET_ERR;
    }
    if (fcntl(*sock_fd, F_SETFL, fcntl(*sock_fd, F_GETFL, 0) | O_NONBLOCK) != 0) {
        return SET_SOCK_OPT_ERR;
    }

    if (listen(*sock_fd, INT_MAX) != 0) {
        return LISTENING_SOCKET_ERR;
    }
    return OK;
}

static void write_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    struct response_t *resp = (struct response_t *)watcher->data;
    int send_res = send_http_response_cb(watcher->fd, resp);
    if (send_res == SOCK_DOESNT_READY_FOR_WRITE) {
        return;
    }
    if (send_res == SOCK_ERR) {
        err_log("error sending response to socket");
    }
    if (resp->body_fd > 0) {
        close(resp->body_fd);
    }
    close(watcher->fd);
    ev_io_stop(loop, watcher);
    free(watcher);
}

static void read_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    struct response_t *resp = calloc(1, sizeof(*resp));
    if (!resp) {
        err_log("error callocing response_t");
        ev_io_stop(loop, watcher);
        close(watcher->fd);
        free(watcher);
        return;
    }
    int resp_res = get_http_response_cb(watcher->fd, resp);

    switch (resp_res) {
        case OK: {
            if (resp->body_fd > 0) {
                close(resp->body_fd);
            }
            close(watcher->fd);
            break;
        }
        case SOCK_DOESNT_READY_FOR_WRITE: {
            struct ev_io *write_client = calloc(1, sizeof(*write_client));
            if (!resp) {
                err_log("error callocing ev_io write_client");
                ev_io_stop(loop, watcher);
                close(watcher->fd);
                free(watcher);
                return;
            }

            write_client->data = resp;
            ev_io_init(write_client, write_cb, watcher->fd, EV_WRITE);
            ev_io_start(loop, write_client);
            break;
        }
        case SOCK_ERR: {
            err_log("error sending response to socket");
            close(watcher->fd);
        }

        default:
            break;
    }

    ev_io_stop(loop, watcher);
    free(watcher);
}

static void accept_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    int client_sd = accept(watcher->fd, NULL, NULL);

    if (client_sd > 0) {
        fcntl(client_sd, F_SETFL, fcntl(client_sd, F_GETFL, 0) | O_NONBLOCK);

        struct timeval timeout = {CLIENT_SOCKET_SEND_TIMEOUT, 0};
        setsockopt(client_sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        timeout.tv_sec = CLIENT_SOCKET_RECV_TIMEOUT;
        setsockopt(client_sd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

        ev_io *client_watcher = calloc(1, sizeof(*client_watcher));
        if (!client_watcher) {
            err_log("error callocing ev_io* client_watcher");
            close(client_sd);
            return;
        }

        ev_io_init(client_watcher, read_cb, client_sd, EV_READ);
        ev_io_start(loop, client_watcher);
    } else if (errno != EAGAIN) {
        err_log("error accepting client connection");
    }
}

int run_server(int cpu_limit, int sock_fd) {
    struct ev_loop *loop = ev_default_loop(EVBACKEND_EPOLL | EVFLAG_FORKCHECK);

    if (loop == NULL) {
        return CREATING_EVLOOP_ERR;
    }

    for (; cpu_limit > 0; --cpu_limit) {
        int pid = fork();
        if (pid == -1) {
            return FORKING_ERR;
        }

        if (pid == 0) {
            add_worker();
            ev_io watcher = {0};
            ev_io_init(&watcher, accept_cb, sock_fd, EV_READ);
            ev_io_start(loop, &watcher);
            ev_run(loop, 0);
            ev_loop_destroy(loop);
            exit(0);
        }

        signal(SIGCHLD, worker_exit_handler_job);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, all_workers_killed_job);
    }

    // TODO след строка вроде не нужна
    // ev_loop_destroy(loop);

    while (true) {
        sleep(INT_MAX);
    }
    return 0;
}
