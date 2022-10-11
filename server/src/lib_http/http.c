#include <string.h>
#include <http.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>
#include <linux/limits.h>

char document_root[MAX_LEN_ROOT_PATH];

int init_document_root(char *new_document_root) {
    char * resolving_path_res = realpath(new_document_root,document_root);
    if (resolving_path_res == NULL) {
        return UNRESOLVING_ROOT_PATH;
    }
    return OK;
}

int get_http_response_cb(int sock_fd, struct  response_t *resp) {
    set_date(resp->date);
    char raw_req[MAX_REQUEST_LEN] = {0};
    read_http_request(sock_fd, raw_req, MAX_REQUEST_LEN);

    struct request_t req = {0};
    if (parse_http_request(raw_req, &req) != 0) {
        resp->status_code = BAD_REQUEST_CODE;
        return send_http_response(sock_fd, resp);;
    }

    if (req.meth == METHOD_NOT_ALLOWED_T) {
        resp->status_code = METHOD_NOT_ALLOWED_CODE;
        return send_http_response(sock_fd, resp);
    }
    resp->method = req.meth;
    if (decode_url(req.url) != 0) {
        resp->status_code = BAD_REQUEST_CODE;
        return send_http_response(sock_fd, resp);;
    }

    char *absolute_static_path = calloc(strlen(req.url) + strlen(document_root) + 1 + strlen("index.html"), sizeof(char));
    absolute_static_path = strcat(absolute_static_path, document_root);
    absolute_static_path = strcat(absolute_static_path, req.url);
    if (req.url[strlen(req.url) - 1] == '/') {
        absolute_static_path = strcat(absolute_static_path, "index.html");
    }
    char resolved_path[PATH_MAX];
    char * resolving_res = realpath(absolute_static_path,resolved_path);
    if (resolving_res == NULL) {
        if (errno != ENOTDIR && req.url[strlen(req.url) - 1] == '/') {
            resp->status_code = FORBIDDEN_CODE;
        } else {
            resp->status_code = NOT_FOUND_CODE;
        }
        free(absolute_static_path);
        return send_http_response(sock_fd, resp);;
    }
    if (strncmp(resolved_path,document_root, strlen(document_root)) != 0) {
        resp->status_code = FORBIDDEN_CODE;
        free(absolute_static_path);
        return send_http_response(sock_fd, resp);;
    }
    FILE *static_file = fopen(absolute_static_path, "r+");
    free(absolute_static_path);
    if (static_file == NULL) {
        if (errno == EACCES) {
            resp->status_code = FORBIDDEN_CODE;
        } else {
            resp->status_code = NOT_FOUND_CODE;
        }
        return send_http_response(sock_fd, resp);;
    }
    resp->body_fd = fileno(static_file);

    resp->content_length = get_content_length(resp->body_fd);
    resp->content_type = get_content_type(req.url);
    resp->status_code = OK_CODE;

//    fclose(static_file); TODO закрывать в рид_цб
    return  send_http_response(sock_fd,resp);
}
