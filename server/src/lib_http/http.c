#include <http.h>
#include <http_inner.h>
#include <linux/limits.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#define FORBIDDEN_ERR (-1)
#define NOT_FOUND_ERR (-2)
#define FILENO_ERR (-3)

char document_root[MAX_LEN_ROOT_PATH];

static int set_fileno(const char *url, int *dest_fd);

int get_http_response_cb(int sock_fd, struct request_t *req) {
    int read_res = read_request(sock_fd, req->raw_req, &(req->req_offset), MAX_REQUEST_LEN);
    if (read_res != OK) {
        if (read_res == SOCK_DOESNT_READY_FOR_READ) {
            return SOCK_DOESNT_READY_FOR_READ;
        }
        if (read_res == SOCK_ERR) {
            return SOCK_ERR;
        }
        if (read_res == TOO_LONG_REQUEST_ERR) {
            req->resp_status_code = BAD_REQUEST_CODE;
            set_http_err_headers(req->resp_status_code, req->header_raw);
            gettimeofday(&req->end_processing_time, NULL);
            return send_response(sock_fd, req);
        }
    }

    gettimeofday(&req->start_processing_time, NULL);
    char request_url[MAX_LEN_URL];

    int parse_req_res = parse_request(req->raw_req, request_url, &req->method);
    if (parse_req_res != OK) {
        if (parse_req_res == UNKNOWN_HTTP_VERSION) {
            req->resp_status_code = BAD_REQUEST_CODE;
        } else {
            req->resp_status_code = METHOD_NOT_ALLOWED_CODE;
        }

        set_http_err_headers(req->resp_status_code, req->header_raw);
        gettimeofday(&req->end_processing_time, NULL);
        return send_response(sock_fd, req);
    }

    if (decode_url(request_url) != OK) {
        req->resp_status_code = BAD_REQUEST_CODE;
        set_http_err_headers(req->resp_status_code, req->header_raw);
        gettimeofday(&req->end_processing_time, NULL);
        return send_response(sock_fd, req);
    }
    int set_fileno_res = set_fileno(request_url, &req->resp_body_fd);

    if (set_fileno_res != OK) {
        if (set_fileno_res == FORBIDDEN_ERR) {
            req->resp_status_code = FORBIDDEN_CODE;
        } else if (set_fileno_res == NOT_FOUND_ERR) {
            req->resp_status_code = NOT_FOUND_CODE;
        } else {
            request_err_log(req->req_id, "error getting fd for requested file");
            req->resp_status_code = INTERNAL_SERVER_ERR_CODE;
        }

        set_http_err_headers(req->resp_status_code, req->header_raw);
        gettimeofday(&req->end_processing_time, NULL);
        return send_response(sock_fd, req);
    }

    req->resp_content_length = get_content_length(req->resp_body_fd);
    req->resp_status_code = OK_CODE;
    set_http_ok_headers(req->resp_status_code, request_url, req->resp_content_length, req->header_raw);
    gettimeofday(&req->end_processing_time, NULL);
    return send_response(sock_fd, req);
}

static int set_fileno(const char *url, int *dest_fd) {
    char absolute_path[MAX_LEN_ABSOLUTE_FILEPATH];
    strncpy(absolute_path, document_root, MAX_LEN_ROOT_PATH);
    strncat(absolute_path, url, MAX_LEN_URL);
    if (is_directory_path(url)) {
        strncat(absolute_path, DEFAULT_FILENAME, strlen(DEFAULT_FILENAME));
    }
    char real_path[MAX_LEN_ABSOLUTE_FILEPATH];
    if (!realpath(absolute_path, real_path)) {
        if (errno != ENOTDIR && is_directory_path(url)) {
            return FORBIDDEN_ERR;
        }
        return NOT_FOUND_ERR;
    }
    if (strncmp(real_path, document_root, strlen(document_root)) != 0) {
        return FORBIDDEN_ERR;
    }
    FILE *file = fopen(real_path, "r+");
    // TODO: тут может быть ошибка, если тесты прошли -> ок
    // if (file == NULL) {
    //     if (errno == EACCES) {
    //         return FORBIDDEN_ERR;
    //     } else {
    //         req->resp_status_code = NOT_FOUND_CODE;
    //     }
    //     return send_response(sock_fd, req);
    //     ;
    // }
    *dest_fd = fileno(file);
    if (*dest_fd == -1) {
        return FILENO_ERR;
    }
    return OK;
}

int is_directory_path(const char *path) {
    return path[strlen(path) - 1] == '/';
}

int init_document_root(const char *new_document_root) {
    if (!realpath(new_document_root, document_root)) {
        return UNRESOLVING_ROOT_PATH;
    }
    return OK;
}

void reset_500_resp(struct request_t *resp) {
    resp->resp_status_code = INTERNAL_SERVER_ERR_CODE;
}