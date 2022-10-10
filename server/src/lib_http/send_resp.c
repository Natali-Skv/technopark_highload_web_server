#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/errno.h>
#include <http_inner.h>
#include <http.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sendfile.h>
#include <sys/types.h>


char headers_template[] = "%s %d %s\r\nDate: %s\r\nServer: web\r\nContent-Length: %d\r\nContent-Type: %s\r\nConnection: %s\r\n\r\n";

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

// если сокет не готов к записи, то возвращаем SOCK_DOESNT ...
//
int send_http_response(int sock_fd, struct response_t *resp ) {
    sprintf(resp->header_raw, headers_template, HTTP1_0, resp->status_code, get_answer_msg(resp->status_code),
            resp->date, resp->content_length, resp->content_type, "Closed");

    size_t header_len = strlen(resp->header_raw);
    ssize_t curr_sent = 0;
    while (resp->headers_offset < header_len) {
        curr_sent = send(sock_fd, resp->header_raw + resp->headers_offset, header_len - resp->headers_offset, 0 );
        if (curr_sent == -1) {
            return errno==EAGAIN?SOCK_DOESNT_READY_FOR_WRITE:SOCK_ERR;
        }
        resp->headers_offset += curr_sent;
    }
    if (resp->status_code != OK_CODE || resp->method == HEAD_T) {
        return OK;
    }

    while (resp->body_sent < resp->content_length) {
        curr_sent = sendfile(sock_fd, resp->body_fd, &resp->body_offset, resp->content_length );
        if (curr_sent == -1) {
            return errno==EAGAIN?SOCK_DOESNT_READY_FOR_WRITE:SOCK_ERR;
        }
        resp->body_sent += curr_sent;
    }
    return OK;
}

int send_http_response_cb(int sock_fd, struct response_t *resp ) {
   size_t header_len = strlen(resp->header_raw);
    ssize_t curr_sent = 0;
    while (resp->headers_offset < header_len) {
        curr_sent = send(sock_fd, resp->header_raw + resp->headers_offset, header_len - resp->headers_offset, 0 );
        if (curr_sent == -1) {
            return errno==EAGAIN?SOCK_DOESNT_READY_FOR_WRITE:SOCK_ERR;
        }
        resp->headers_offset += curr_sent;
    }
    if (resp->status_code != OK_CODE || resp->method == HEAD_T) {
        return OK;
    }

    while (resp->body_sent < resp->content_length) {
        curr_sent = sendfile(sock_fd, resp->body_fd, &resp->body_offset, resp->content_length );
        if (curr_sent == -1) {
            return errno==EAGAIN?SOCK_DOESNT_READY_FOR_WRITE:SOCK_ERR;
        }
        resp->body_sent += curr_sent;
    }
    return OK;
}

