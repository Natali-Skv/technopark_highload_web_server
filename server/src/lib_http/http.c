#include <string.h>
#include <http.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>
#include <linux/limits.h>

char *document_root;

void init_document_root(char *new_document_root) {
    document_root = new_document_root;
}

void get_http_response_cb(int sock_fd) {
    struct response_t resp = {0};
    set_date(resp.date);
    char raw_req[MAX_REQUEST_LEN] = {0};
    read_http_request(sock_fd, raw_req, MAX_REQUEST_LEN);

    struct request_t req = {0};
    if (parse_http_request(raw_req, &req) != 0) {
        err_log_code("Error parsing client request", errno);
        resp.status_code = BAD_REQUEST_CODE;
        send_http_response(sock_fd, &resp);
        return;
    }

    if (req.method == METHOD_NOT_ALLOWED_T) {
        resp.status_code = METHOD_NOT_ALLOWED_CODE;
        send_http_response(sock_fd, &resp);
        return;
    }
    resp.method = req.method;
    if (decode_url(req.url) != 0) {
        resp.status_code = BAD_REQUEST_CODE;
        send_http_response(sock_fd, &resp);
        return;
    }

    printf("%lu\n",strlen(req.url));
    printf("%s\n",document_root);
    printf("%lu\n",strlen(document_root));
    printf("%lu\n",strlen("index.html"));
    char *absolute_static_path = calloc(strlen(req.url) + strlen(document_root) + 1 + strlen("index.html"), sizeof(char));
    absolute_static_path = strcat(absolute_static_path, document_root);
    absolute_static_path = strcat(absolute_static_path, req.url);
    printf("\n%s\n",absolute_static_path);
    if (req.url[strlen(req.url) - 1] == '/') {
        absolute_static_path = strcat(absolute_static_path, "index.html");
    }
    char resolved_path[PATH_MAX];
    char * resolving_res = realpath(absolute_static_path,resolved_path);
    printf("%d\n",__LINE__);
    printf("\n%s\n",absolute_static_path);
    printf("%d\n",__LINE__);
    printf("\n%s\n",resolved_path);
    printf("%d\n",__LINE__);
    if (resolving_res == NULL) {
        printf("!!!!! %d\n", errno);
        if (errno != ENOTDIR && req.url[strlen(req.url) - 1] == '/') {
            resp.status_code = FORBIDDEN_CODE;
        } else {
            resp.status_code = NOT_FOUND_CODE;
        }
        printf("%d\n",__LINE__);
        printf("%d\n",__LINE__);
        send_http_response(sock_fd, &resp);
        printf("%d\n",__LINE__);
        free(absolute_static_path);
        return;
    }
    printf("%d\n",__LINE__);
    if (strncmp(resolved_path,document_root, strlen(document_root)) != 0) {
        printf("%d\n",__LINE__);
        resp.status_code = FORBIDDEN_CODE;
        send_http_response(sock_fd, &resp);
        free(absolute_static_path);
        return;
    }
    FILE *static_file = fopen(absolute_static_path, "r+");
    printf("%d\n",__LINE__);
    if (static_file == NULL) {
        printf("%d\n",__LINE__);
        if (errno == EACCES) {
            printf("%d\n",__LINE__);
            resp.status_code = FORBIDDEN_CODE;
            printf("%d\n",__LINE__);
        } else {
            printf("%d\n",__LINE__);
            resp.status_code = NOT_FOUND_CODE;
        }
        printf("%d\n",__LINE__);
        send_http_response(sock_fd, &resp);
        free(absolute_static_path);
        fclose(static_file);
        return;
    }
    resp.body_fd = fileno(static_file);

    resp.content_length = get_content_length(resp.body_fd);
    // тут
    printf("%s\n",req.url);
    resp.content_type = get_content_type(req.url);
    printf("%d\n",__LINE__);
    resp.status_code = OK_CODE;
    printf("%d\n",__LINE__);

    if (send_http_response(sock_fd,&resp) !=0 ) {
        printf("%d\n",__LINE__);
        err_log_code("Error sending  client request", errno);
    }
    printf("%d\n",__LINE__);

    fclose(static_file);
    printf("%d\n",__LINE__);
    printf("%s\n",absolute_static_path);
    free(absolute_static_path);
}



//void get_http_response_cb(int sock_fd) {
//    struct response_t resp = {0};
//    set_date(resp.date);
//
//
//    char * buf = calloc(1000, sizeof(char));
//    char * tmp_buf = calloc(800, sizeof(char));
//    int rcvd = 0;
//    while(buf[rcvd] != '\n') {
//        int rvd = recv(sock_fd, tmp_buf, sizeof(tmp_buf) - 1, MSG_DONTWAIT);
//        if (rvd == 0)
//            break;
//        if (rvd != -1) {
//            rcvd += rvd;
//            buf = strcat(buf, tmp_buf);
//            memset(tmp_buf, 0, strlen(tmp_buf));
//        }
//        if (rvd == -1 && errno != EAGAIN) {
//            free(buf);
//            free(tmp_buf);
//            return;
//        } else if (rvd == -1) {
//            break;
//        }
//    }
//
//    struct request_t req = {0};
//    if (parse_http_request(buf, &req) != 0) {
//        err_log_code("Error parsing client request", errno);
//        resp.status_code = BAD_REQUEST_CODE;
//        send_http_response(sock_fd, &resp);
//        return;
//    }
//
//    if (req.method == METHOD_NOT_ALLOWED_T) {
//        resp.status_code = METHOD_NOT_ALLOWED_CODE;
//        send_http_response(sock_fd, &resp);
//        return;
//    }
//    resp.method = req.method;
//    if (decode_url(req.url) != 0) {
//        resp.status_code = BAD_REQUEST_CODE;
//        send_http_response(sock_fd, &resp);
//        return;
//    }
//
//    printf("%lu\n",strlen(req.url));
//    printf("%s\n",document_root);
//    printf("%lu\n",strlen(document_root));
//    printf("%lu\n",strlen("index.html"));
//    char *absolute_static_path = calloc(strlen(req.url) + strlen(document_root) + 1 + strlen("index.html"), sizeof(char));
//    absolute_static_path = strcat(absolute_static_path, document_root);
//    absolute_static_path = strcat(absolute_static_path, req.url);
//    printf("\n%s\n",absolute_static_path);
//    if (req.url[strlen(req.url) - 1] == '/') {
//        absolute_static_path = strcat(absolute_static_path, "index.html");
//    }
//    char resolved_path[PATH_MAX];
//    char * resolving_res = realpath(absolute_static_path,resolved_path);
//    printf("%d\n",__LINE__);
//    printf("\n%s\n",absolute_static_path);
//    printf("%d\n",__LINE__);
//    printf("\n%s\n",resolved_path);
//    printf("%d\n",__LINE__);
//    if (resolving_res == NULL) {
//        printf("!!!!! %d\n", errno);
//        if (errno != ENOTDIR && req.url[strlen(req.url) - 1] == '/') {
//            resp.status_code = FORBIDDEN_CODE;
//        } else {
//            resp.status_code = NOT_FOUND_CODE;
//        }
//        printf("%d\n",__LINE__);
//        printf("%d\n",__LINE__);
//        send_http_response(sock_fd, &resp);
//        printf("%d\n",__LINE__);
//        free(absolute_static_path);
//        return;
//    }
//    printf("%d\n",__LINE__);
//    if (strncmp(resolved_path,document_root, strlen(document_root)) != 0) {
//        printf("%d\n",__LINE__);
//        resp.status_code = FORBIDDEN_CODE;
//        send_http_response(sock_fd, &resp);
//        free(absolute_static_path);
//        return;
//    }
//    FILE *static_file = fopen(absolute_static_path, "r+");
//    printf("%d\n",__LINE__);
//    if (static_file == NULL) {
//        printf("%d\n",__LINE__);
//        if (errno == EACCES) {
//            printf("%d\n",__LINE__);
//            resp.status_code = FORBIDDEN_CODE;
//            printf("%d\n",__LINE__);
////        } else if (errno == ENOENT &&
////                   strcmp(absolute_static_path + strlen(absolute_static_path) - strlen("index.html"), "index.html") == 0) {
////            printf("%d\n",__LINE__);
////            resp.status_code = FORBIDDEN_CODE;
//        } else {
//            printf("%d\n",__LINE__);
//            resp.status_code = NOT_FOUND_CODE;
//        }
//        printf("%d\n",__LINE__);
//        send_http_response(sock_fd, &resp);
//        free(absolute_static_path);
//        fclose(static_file);
//        return;
//    }
//    resp.body_fd = fileno(static_file);
//
//    resp.content_length = get_content_length(resp.body_fd);
//    // тут
//    printf("%s\n",req.url);
//    resp.content_type = get_content_type(req.url);
//    printf("%d\n",__LINE__);
//    resp.status_code = OK_CODE;
//    printf("%d\n",__LINE__);
//
//    if (send_http_response(sock_fd,&resp) !=0 ) {
//        printf("%d\n",__LINE__);
//        err_log_code("Error sending  client request", errno);
//    }
//    printf("%d\n",__LINE__);
//
//    fclose(static_file);
//    printf("%d\n",__LINE__);
//    printf("%s\n",absolute_static_path);
//    free(absolute_static_path);
//}




