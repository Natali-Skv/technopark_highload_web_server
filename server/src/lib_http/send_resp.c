#include <string.h>
#include <http.h>
#include <sys/socket.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sendfile.h>

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
        {NULL, NULL},
};

int get_static_fd(char *file_path) {
    FILE *f = fopen(file_path, "r+");
    if (f == NULL) {
        return -1;
    }
    return fileno(f);
}

size_t get_content_length(int fd) {
    struct stat statistics;
    if (fstat(fd, &statistics) == -1) {
        return 0;
    }
    return statistics.st_size;
}

const char *get_content_type(char *filename) {
// найти точку в конце
// сравнить
    size_t filename_len = strlen(filename);
    char extention[5] = {0};
    size_t dot_pos = 0;
    // найти последнюю точку
    for (size_t pos = 0; filename[pos] != '\0'; pos++) {
        if (filename[pos] == '.') {
            dot_pos = pos;
        }
    }
    for (int i = 0; i < sizeof(content_type_table); i++) {
        if (strcmp(filename + dot_pos, content_type_table[i].extension)) {
            return content_type_table[i].content_type;
        }
    }
    return NULL;
}

void set_date(char *date_dst) {
    long int s_time;
    struct tm m_time;

    s_time = time(NULL);
    char buf[26] = "";
    localtime_r(&s_time, &m_time);

    date_dst = calloc(sizeof("Mon, 27 Jul 2009 12:28:53 GMT"), sizeof(char));
    sprintf(date_dst, "%s GMT", asctime_r(&m_time, buf));
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
    }
    return "";
}


int send_http_response(int sock_fd, struct response_t *resp) {
    char *template = "%s %d %s\r\nDate: %s\r\nServer: web\r\nContent-Length: %d\r\nContent-Type: %s\r\nConnection: %s\r\n\r\n";
    char buf[150] = {0};
    sprintf(buf, template, HTTP1_0, resp->status_code, get_answer_msg(resp->status_code),
            resp->date, resp->content_length, resp->content_type, "Closed");

    size_t left = strlen(buf);
    ssize_t sent = 0;
    while (sent < left) {
        long snt = send(sock_fd, buf + sent, strlen(buf) - sent, 0);
        if (snt == -1) {
            free(buf);
            return -1;
        }
        sent += snt;
    }
    if (resp->status_code == OK_CODE && resp->method != HEAD_T) {
        int fd = resp->body_fd;
        int sd = sock_fd;

        long long file_bytes_sent = resp->content_length;
        long long sent = 0;

        int res = -1;
        while(1) {
#if defined(__linux__)
            res = sendfile(sd, fd, 0, resp->content_length);
#elif defined(__APPLE__)
            res = sendfile(fd, sd, 0, &file_bytes_sent, NULL,  0);
#endif
            sent += file_bytes_sent;
            while (sent < resp->content_length) {
                res = sendfile(fd, sd, sent, &file_bytes_sent);
                sent += file_bytes_sent;
            }
            if (res == 0) {
                break;
            }
            if (res == -1 && errno != EAGAIN) {
                break;
            }
        }
    }
    // отправлять файл через sendfile
    // отправить в сокет
}
