#ifndef WEB_SERVER_HTTP_INNER_H
#define WEB_SERVER_HTTP_INNER_H

#include <http.h>

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
#define INTERNAL_SERVER_ERR_CODE 500

#define OK_STATUS "OK"
#define NOT_FOUND_STATUS "NOT FOUND"
#define FORBIDDEN_STATUS "FORBIDDEN"
#define METHOD_NOT_ALLOWED_STATUS "METHOD NOT ALLOWED"
#define BAD_REQUEST_STATUS "BAD REQUEST"

#define UNKNOWN_HTTP_VERSION (-1)
#define FSTAT_ERR (-2)

int decode_url(char *url);
int is_directory_path(const char *path);
int parse_request(const char *raw_req,char *dst_url, enum rest_method_t *method);
int read_request(int socket_fd, char *req, size_t *offset, size_t max_req_len);
void set_http_err_headers(int status_code, char * headers_raw);
void set_http_ok_headers(int status_code,const char * filename, size_t content_length, char * dst_headers_raw);
ssize_t get_content_length(int fd);
const char *get_content_type(const char *filename);
void set_date(char * date_dst);
int send_response(int sock_fd, struct request_t *resp);

#endif //WEB_SERVER_HTTP_INNER_H
