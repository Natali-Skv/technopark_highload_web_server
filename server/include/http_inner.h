#ifndef WEB_SERVER_HTTP_INNER_H
#define WEB_SERVER_HTTP_INNER_H

#define MAX_REQUEST_LEN 1000
#define METHOD_MAX_LEN 10
#define HTTP_VERSION_MAX_LEN 10
#define URL_MAX_LEN 990
#define DATE_MAX_LEN 40

#define GET "GET"
#define HEAD "HEAD"
#define HTTP1_0 "HTTP/1.0"
#define HTTP1_1 "HTTP/1.1"
#define HTTP2_0 "HTTP/2.0"

#define OK_CODE 200
#define NOT_FOUND_CODE 404
#define FORBIDDEN_CODE 403
#define METHOD_NOT_ALLOWED_CODE 405
#define BAD_REQUEST_CODE 400

#define OK_STATUS "OK"
#define NOT_FOUND_STATUS "NOT FOUND"
#define FORBIDDEN_STATUS "FORBIDDEN"
#define METHOD_NOT_ALLOWED_STATUS "METHOD NOT ALLOWED"
#define BAD_REQUEST_STATUS "BAD REQUEST"

enum request_method_t {
    GET_T,
    HEAD_T,
    METHOD_NOT_ALLOWED_T,
};

enum http_version_t {
    HTTP1_0_T,
    HTTP1_1_T,
    HTTP2_0_T,
    UNKNOWN_HTTP_T,
};

struct request_t {
    enum request_method_t method;
    enum http_version_t version;
    char url[URL_MAX_LEN];
};

struct response_t {
    int status_code;
    size_t content_length;
    int body_fd;
    const char * content_type;
    char date [DATE_MAX_LEN];
    enum request_method_t method;
};

int decode_url(char *url);
int parse_http_request(char *raw_req, struct request_t *req);
void read_http_request(int socket_fd, char *req, size_t max_req_len);
size_t get_content_length(int fd);
const char *get_content_type(char *filename);
void set_date(char * date_dst);
int send_http_response(int sock_fd, struct response_t *resp);

#endif //WEB_SERVER_HTTP_INNER_H
