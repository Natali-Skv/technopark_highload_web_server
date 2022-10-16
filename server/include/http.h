#ifndef WEB_SERVER_HTTP_H
#define WEB_SERVER_HTTP_H

#include <server.h>
#include <sys/types.h>
#include <sys/time.h>

#define SOCK_DOESNT_READY_FOR_WRITE (-1)
#define SOCK_DOESNT_READY_FOR_READ (-2)
#define SOCK_ERR (-3)
#define PARSING_REQ_ERR (-4)
#define OK 0

#define UNRESOLVING_ROOT_PATH (-1)

enum rest_method_t {
    GET_T,
    HEAD_T,
    METHOD_NOT_ALLOWED_T,
};

struct request_t {
    char raw_req[MAX_REQUEST_LEN];
    unsigned int req_id;
    size_t req_offset;
    size_t resp_headers_offset;
    off_t resp_body_offset;
    size_t resp_body_sent;
    int resp_status_code;
    size_t resp_content_length;
    int resp_body_fd;
    enum rest_method_t method;
    char header_raw[MAX_LEN_HEADERS_VALUES];
    size_t header_len;
    struct timeval start_full_time; 
    struct timeval end_full_time;
    struct timeval start_processing_time; 
    struct timeval end_processing_time;
};

int init_document_root(const char *new_document_root);
int get_http_response_cb(int sock_fd, struct request_t *resp);
int send_response(int sock_fd, struct request_t *req );

#endif  // WEB_SERVER_HTTP_H
