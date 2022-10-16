#include <http.h>
#include <http_inner.h>
#include <logger.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static const struct table_entry {
    const char *extension;
    const char *content_type;
} content_type_table[] = {
    {"txt", "text/plain"},
    {"c", "text/plain"},
    {"h", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/htm"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"pdf", "application/pdf"},
    {"ps", "application/postscript"},
    {"js", "application/javascript"},
    {"swf", "application/x-shockwave-flash"},
    {NULL, NULL},
};

ssize_t get_content_length(int fd) {
    struct stat statistics;
    if (fstat(fd, &statistics) == -1) {
        return FSTAT_ERR;
    }
    return statistics.st_size;
}

const char *get_content_type(const char *filename) {
    if (is_directory_path(filename)) {
        filename = DEFAULT_FILENAME;
    }
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

void set_http_err_headers(int status_code, char *dst_headers_raw) {
    char headers_template[] = "%s %d %s\r\nDate: %s\r\nServer: web\r\nContent-Length: 0\r\nContent-Type: \r\nConnection: Closed\r\n\r\n";
    char date[MAX_LEN_DATE];
    set_date(date);
    sprintf(dst_headers_raw, headers_template, HTTP1_0, status_code, get_answer_msg(status_code), date);
}

void set_http_ok_headers(int status_code, const char *filename, size_t content_length, char *dst_headers_raw) {
    char headers_template[] = "%s %d %s\r\nDate: %s\r\nServer: web\r\nContent-Length: %d\r\nContent-Type: %s\r\nConnection: Closed\r\n\r\n";
    char date[MAX_LEN_DATE];
    set_date(date);
    sprintf(dst_headers_raw, headers_template, HTTP1_0, status_code, get_answer_msg(status_code), date, content_length, get_content_type(filename));
}

int send_response(int sock_fd, struct request_t *req) {
 
    size_t header_len = strlen(req->header_raw);
    ssize_t curr_sent = 0;
 
    while (req->resp_headers_offset < header_len) {
 
        curr_sent = send(sock_fd, req->header_raw + req->resp_headers_offset, header_len - req->resp_headers_offset, 0);
        if (curr_sent == -1) {
 
            if (errno == EAGAIN) {
 
                return SOCK_DOESNT_READY_FOR_READ;
            }
 
            request_err_log(req->req_id, "1 error sending response");
            return SOCK_ERR;
        }
 
        req->resp_headers_offset += curr_sent;
    }
    if (req->resp_status_code != OK_CODE || req->method == HEAD_T) {
 
        return OK;
    }
    // TODO remove
    // return OK;

 
    while (req->resp_body_sent < req->resp_content_length) {
 
        curr_sent = sendfile(sock_fd, req->resp_body_fd, &req->resp_body_offset, req->resp_content_length);
 
        if (curr_sent == -1) {
 
            if (errno == EAGAIN) {
 
                return SOCK_DOESNT_READY_FOR_READ;
            }
 
            request_err_log(req->req_id, "2 error sending response");
 
            return SOCK_ERR;
        }
 
        req->resp_body_sent += curr_sent;
    }
 
    return OK;
}
