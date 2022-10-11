#ifndef WEB_SERVER_HTTP_H
#define WEB_SERVER_HTTP_H

#include <sys/types.h>

#define MAX_LEN_ROOT_PATH 200
#define DATE_MAX_LEN 40
#define HEADERS_VALUES_MAX_LEN 150

#define SOCK_DOESNT_READY_FOR_WRITE (-1)
#define SOCK_ERR (-2)
#define OK 0

#define UNRESOLVING_ROOT_PATH (-1)

enum request_method_t {
    GET_T,
    HEAD_T,
    METHOD_NOT_ALLOWED_T,
};

struct response_t {
    size_t headers_offset;
    off_t body_offset;
    size_t body_sent;
    int status_code;
    size_t content_length;
    int body_fd;
    const char * content_type;
    char date [DATE_MAX_LEN];
    enum request_method_t method;
    char header_raw[HEADERS_VALUES_MAX_LEN];
};

int init_document_root(char *new_document_root);
int get_http_response_cb(int sock_fd, struct  response_t *resp);
int send_http_response_cb(int sock_fd, struct response_t *resp );

#endif //WEB_SERVER_HTTP_H
