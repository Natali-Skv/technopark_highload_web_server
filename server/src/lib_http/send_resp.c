#include <string.h>
#include <http.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>


static const struct table_entry {
    const char *extension;
    const char *content_type;
} content_type_table[] = {
        {"txt",  "text/plain"},
        {"c",    "text/plain"},
        {"h",    "text/plain"},
        {"html", "text/html"},
        {"htm",  "text/htm"},
        {"css",  "text/css"},
        {"gif",  "image/gif"},
        {"jpg",  "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png",  "image/png"},
        {"pdf",  "application/pdf"},
        {"ps",   "application/postscript"},
        {"js",   "application/javascript"},
        {"swf",  "application/x-shockwave-flash"},
        {NULL, NULL},
};

size_t get_content_length(int fd) {
    struct stat statistics;
    if (fstat(fd, &statistics) == -1) {
        return 0;
    }
    return statistics.st_size;
}

const char *get_content_type(char *filename) {
    size_t dot_pos = 0;
    for (size_t pos = 0; filename[pos] != '\0'; pos++) {
        if (filename[pos] == '.') {
            dot_pos = pos;
        }
    }
    for (int i = 0; i < sizeof(content_type_table) / sizeof(content_type_table[0]) - 1; i++) {
        if (strcmp(filename + dot_pos + 1, content_type_table[i].extension) == 0) {
            return content_type_table[i].content_type;
        }
    }
    return "";
}

void set_date(char *date_dst) {
    long int s_time;
    struct tm m_time;

    s_time = time(NULL);
    localtime_r(&s_time, &m_time);
    asctime_r(&m_time, date_dst);
    date_dst[strlen(date_dst) - 2] = '\0';
}

const char *get_answer_msg(int http_status_code) {
    switch (http_status_code) {
        case OK_CODE:
            return OK_STATUS;
        case METHOD_NOT_ALLOWED_CODE:
            return METHOD_NOT_ALLOWED_STATUS;
        case NOT_FOUND_CODE:
            return NOT_FOUND_STATUS;
        case FORBIDDEN_CODE:
            return FORBIDDEN_STATUS;
        case BAD_REQUEST_CODE:
            return BAD_REQUEST_STATUS;
    }
    return "";
}

int send_http_response(int sock_fd, struct response_t *resp) {
    char template[] = "%s %d %s\r\nDate: %s\r\nServer: web\r\nContent-Length: %d\r\nContent-Type: %s\r\nConnection: %s\r\n\r\n";
    char buf[150] = {0};
    sprintf(buf, template, HTTP1_0, resp->status_code, get_answer_msg(resp->status_code),
            resp->date, resp->content_length, resp->content_type, "Closed");

    size_t left = strlen(buf);
    size_t sent = 0;
    while (sent < left) {
        long snt = send(sock_fd, buf + sent, strlen(buf) - sent, 0);
        if (snt == -1) {
            return -1;
        }
        sent += snt;
    }
    if (resp->status_code != OK_CODE || resp->method == HEAD_T) {
        return 0;
    }

    sent = 0;
    off_t offset = 0;
    while (sent < resp->content_length) {
        size_t res = sendfile(sock_fd, resp->body_fd, &offset, resp->content_length);
        if (res == -1 && errno != EAGAIN) {
            return -1;
        }
        sent += res;
    }
    return 0;
}

