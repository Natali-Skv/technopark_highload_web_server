#include <errno.h>
#include <http.h>
#include <logger.h>
#include <server.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

time_t now;
FILE *server_log_file = NULL;
FILE *access_log_file = NULL;
char *a_log_buf = NULL;
char *a_log_offset = NULL;
size_t a_log_buf_len = MAX_LEN_LOG_MSG * LEN_LOG_BUF;

static unsigned long get_us_diff(const struct timeval *start, const struct timeval *end);

int init_logger(const char *server_log_path_prefix, const char *access_log_path_prefix) {
    a_log_buf = (char *)malloc(sizeof(char) * a_log_buf_len);
    a_log_offset = a_log_buf;

    char pid_str[10] = {0};
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());

    if (!server_log_path_prefix || !server_log_path_prefix[0]) {
        server_log_file = stderr;
    } else {
        char server_log_filename[MAX_LEN_SERVER_LOG_PATH + sizeof(pid_str)];
        strncpy(server_log_filename, server_log_path_prefix, sizeof(server_log_filename));
        strncat(server_log_filename, pid_str, sizeof(server_log_filename));
        server_log_file = fopen(server_log_filename, "w");
        if (server_log_file == NULL) {
            return OPENING_SERVER_LOG_FILE_ERR;
        }
    }

    if (!access_log_path_prefix || !access_log_path_prefix[0]) {
        access_log_file = stdout;
    } else {
        char access_log_filename[MAX_LEN_ACCESS_LOG_PATH + sizeof(pid_str)];
        strncpy(access_log_filename, access_log_path_prefix, sizeof(access_log_filename));
        strncat(access_log_filename, pid_str, sizeof(access_log_filename));
        access_log_file = fopen(access_log_filename, "w");
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
    fwrite(a_log_buf, sizeof(char), a_log_offset - a_log_buf, access_log_file);
    if (!fclose(server_log_file)) {
        return CLOSING_SERVER_LOG_FILE_ERR;
    }
    if (!fclose(access_log_file)) {
        return CLOSING_ACCESS_LOG_FILE_ERR;
    }
    return OK;
}

void err_log(const char *fmt, ...) {
    time(&now);
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LEN_LOG_MSG];
    snprintf(full_fmt, MAX_LEN_LOG_MSG, "[ERROR] %s: %d: %s: %s\n", strtok(ctime(&now), "\n"), getpid(), fmt,
             strerror(errno));
    vfprintf(server_log_file, full_fmt, args);
}

void info_log(const char *fmt, ...) {
    time(&now);
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LEN_LOG_MSG];
    snprintf(full_fmt, MAX_LEN_LOG_MSG, "[INFO] %s: %d: %s\n", strtok(ctime(&now), "\n"), getpid(), fmt);
    vfprintf(server_log_file, full_fmt, args);
}

void request_err_log(unsigned int req_id, const char *fmt, ...) {
    time(&now);
    va_list args;
    va_start(args, fmt);
    char full_fmt[MAX_LEN_LOG_MSG];
    snprintf(full_fmt, MAX_LEN_LOG_MSG, "[R_ERROR] %d-%u: %s: %s: %s\n", getpid(), req_id, strtok(ctime(&now), "\n"), fmt, strerror(errno));
    vfprintf(server_log_file, full_fmt, args);
}

void access_log(const struct request_t *req) {
    time(&now);
    a_log_offset += sprintf(a_log_offset, "%d-%u: %s: %s: %d: %lu: %.lu\n", getpid(), req->req_id, strtok(ctime(&now), "\n"), req->raw_req, req->resp_status_code, get_us_diff(&req->start_processing_time, &req->end_processing_time), get_us_diff(&req->start_full_time, &req->end_full_time)) * sizeof(char);
    if (a_log_buf_len - (a_log_offset - a_log_buf) < MAX_LEN_LOG_MSG) {
        fwrite(a_log_buf, sizeof(char), a_log_offset - a_log_buf, access_log_file);
        a_log_offset = a_log_buf;
    }
}

static unsigned long get_us_diff(const struct timeval *start, const struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec;
}
