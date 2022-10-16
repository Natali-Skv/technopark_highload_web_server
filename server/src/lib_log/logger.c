#include <errno.h>
#include <logger.h>
#include <server.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <http.h>

time_t timer;
FILE *server_log_file = NULL;
FILE *access_log_file = NULL;

static unsigned long get_us_diff(const struct timeval *start,const struct timeval *end);

int init_logger(const char *server_log, const char *access_log) {
    time(&timer);
    if (!server_log || !server_log[0]) {
        server_log_file = stderr;
    } else {
        server_log_file = fopen(server_log, "w");
        if (server_log_file == NULL) {
            return OPENING_SERVER_LOG_FILE_ERR;
        }
    }
    if (!access_log || !access_log[0]) {
        access_log_file = stdout;
    } else {
        access_log_file = fopen(access_log, "w");
        if (access_log_file == NULL) {
            if (server_log_file != stderr) {
                fclose(server_log_file);
            }
            return OPENING_ACCESS_LOG_FILE_ERR;
        }
    }
    return OK;
}

int destruct_logger() {
    if (!fclose(server_log_file)) {
        return CLOSING_SERVER_LOG_FILE_ERR;
    }
    if (!fclose(access_log_file)) {
        return CLOSING_ACCESS_LOG_FILE_ERR;
    }
    return OK;
}

void err_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LOG_MSG_LEN];
    snprintf(full_fmt, MAX_LOG_MSG_LEN, "[ERROR] %s: %d: %s: %s\n", strtok(ctime(&timer), "\n"), getpid(), fmt,
             strerror(errno));
    vfprintf(server_log_file, full_fmt, args);
}

void info_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LOG_MSG_LEN];
    snprintf(full_fmt, MAX_LOG_MSG_LEN, "[INFO] %s: %d: %s\n", strtok(ctime(&timer), "\n"), getpid(), fmt);
    vfprintf(server_log_file, full_fmt, args);
}

void request_err_log(unsigned int req_id, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LOG_MSG_LEN];
    snprintf(full_fmt, MAX_LOG_MSG_LEN, "[R_ERROR] %d-%u: %s: %s: %s\n", getpid(), req_id, strtok(ctime(&timer), "\n"), fmt, strerror(errno));
    vfprintf(server_log_file, full_fmt, args);
}

void access_log(const struct request_t *req) {
    fprintf(access_log_file, "%d-%u: %s: %s: %d: %lu: %.lu\n", getpid(), req->req_id, strtok(ctime(&timer), "\n"), req->raw_req,  req->resp_status_code, get_us_diff(&req->start_processing_time, &req->end_processing_time), get_us_diff(&req->start_full_time, &req->end_full_time));
}

static unsigned long get_us_diff(const struct timeval *start,const struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec;
}
