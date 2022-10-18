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
    struct request_t *req;
};

int set_socket(int port, int *sock_fd);
int run_server(int sock_fd, struct config_t *cfg);

int main(int argc, char *argv[]) {
    struct config_t cfg = {0};
    if (set_config(argc, argv, &cfg) != OK) {
        return 0;
    }
    set_config_default_values(&cfg);

    if (init_document_root(cfg.document_root) == UNRESOLVING_ROOT_PATH) {
        printf("error resolving root path: %s", strerror(errno));
        return 0;
    }

    int sock_fd = -1;
    int set_socket_res = set_socket(cfg.port, &sock_fd);
    if (set_socket_res == CREATING_SOCKET_ERR) {
        printf("error creating socket: %s", strerror(errno));
        return 0;
    } else if (set_socket_res == SET_SOCK_OPT_ERR) {
        printf("error setting socket options: %s", strerror(errno));
        return 0;
    } else if (set_socket_res == BINDING_SOCKET_ERR) {
        printf("error bilding socket: %s", strerror(errno));
        return 0;
    } else if (set_socket_res == LISTENING_SOCKET_ERR) {
        printf("error listening socket: %s", strerror(errno));
        return 0;
    }

    init_sig_handler(sock_fd);

    if (run_server(sock_fd, &cfg) != 0) {
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
        close(*sock_fd);
        return BINDING_SOCKET_ERR;
    }
    if (fcntl(*sock_fd, F_SETFL, fcntl(*sock_fd, F_GETFL, 0) | O_NONBLOCK) != 0) {
        close(*sock_fd);
        return SET_SOCK_OPT_ERR;
    }

    if (listen(*sock_fd, INT_MAX) != 0) {
        close(*sock_fd);
        return LISTENING_SOCKET_ERR;
    }
    return OK;
}

static void write_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    struct request_t *req = (struct request_t *)watcher->data;
    int send_res = send_response(watcher->fd, req);
    if (send_res == SOCK_DOESNT_READY_FOR_WRITE) {
        return;
    }
    if (send_res == SOCK_ERR) {
        request_err_log(req->req_id, "error sending response to socket");
    }
    if (req->resp_body_fd > 0) {
        close(req->resp_body_fd);
    }
    gettimeofday(&req->end_full_time, NULL);
    access_log(req);
    close(watcher->fd);
    free(req);
    ev_io_stop(loop, watcher);
    free(watcher);
}

static void read_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    struct request_t *req = (struct request_t *)watcher->data;
    gettimeofday(&req->start_full_time, NULL);
    int resp_res = get_http_response_cb(watcher->fd, req);

    switch (resp_res) {
        case OK: {
            break;
        }
        case SOCK_DOESNT_READY_FOR_READ: {
            return;
        }
        case SOCK_DOESNT_READY_FOR_WRITE: {
            struct ev_io *write_client = calloc(1, sizeof(*write_client));
            if (!req) {
                err_log("error callocing ev_io write_client");
                break;
            }

            write_client->data = req;
            ev_io_init(write_client, write_cb, watcher->fd, EV_WRITE);
            ev_io_start(loop, write_client);
            ev_io_stop(loop, watcher);
            free(watcher);
            return;
        }
        case SOCK_ERR: {
            request_err_log(req->req_id, "error with socket");
            break;
        }
    }

    if (req->resp_body_fd > 0) {
        close(req->resp_body_fd);
    }

    close(watcher->fd);

    gettimeofday(&req->end_full_time, NULL);
    access_log(req);

    ev_io_stop(loop, watcher);
    free(req);
    free(watcher);
}

static void accept_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    static unsigned int req_id = 0;
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

        struct request_t *req = calloc(1, sizeof(*req));
        if (!client_watcher) {
            err_log("error callocing request_t *");
            close(client_sd);
            return;
        }
        req->req_id = ++req_id;
        client_watcher->data = req;
        ev_io_init(client_watcher, read_cb, client_sd, EV_READ);
        ev_io_start(loop, client_watcher);
    } else if (errno != EAGAIN) {
        err_log("error accepting client connection");
    }
}

int run_server(int sock_fd, struct config_t *cfg) {
    struct ev_loop *loop = ev_default_loop(EVBACKEND_EPOLL | EVFLAG_FORKCHECK);

    signal(SIGCHLD, worker_exit_handler_job);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, all_workers_killed_job);

    if (loop == NULL) {
        return CREATING_EVLOOP_ERR;
    }

    for (; cfg->cpu_limit > 0; --cfg->cpu_limit) {
        int pid = fork();
        if (pid == -1) {
            return FORKING_ERR;
        }

        if (pid == 0) {
            int init_logger_res = init_logger(cfg->server_log_path, cfg->access_log_path_prefix);
            if (init_logger_res == OPENING_SERVER_LOG_FILE_ERR) {
                printf("error opening server log file: %s", strerror(errno));
                return 0;
            }
            if (init_logger_res == OPENING_ACCESS_LOG_FILE_ERR) {
                printf("error opening access log file: %s", strerror(errno));
                return 0;
            }

            info_log("process with pid %d, root directory %s started", getpid(), cfg->document_root);
            ev_io watcher = {0};
            ev_io_init(&watcher, accept_cb, sock_fd, EV_READ);
            ev_io_start(loop, &watcher);
            ev_run(loop, 0);
            ev_loop_destroy(loop);
            destruct_logger();
            exit(0);
        }

        add_worker();
    }

    while (true) {
        sleep(INT_MAX);
    }
    return 0;
}
