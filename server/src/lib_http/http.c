#include <string.h>
#include <http.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>
#include <unistd.h>

char *document_root;

void init_document_root(char *new_document_root) {
    document_root = new_document_root;
}

void get_http_response_cb(int sock_fd) {
    char raw_req[MAX_REQUEST_LEN] = {0};
    if (read_http_request(sock_fd, raw_req, MAX_REQUEST_LEN) != 0) {
        err_log_code("Error reading client request", errno);
        // TODO: отправить бед реквест
        return;
    }
    printf("raw request: %s", raw_req);
    struct request_t req = {0};
    if (parse_http_request(raw_req, &req) != 0) {
        err_log_code("Error parsing client request", errno);
        // TODO: отправить бед реквест
        return;
    }
    printf("%d\n", __LINE__);
    printf("parsed request: %u, %s, %u", req.method, req.url, req.version);
    printf("%d\n", __LINE__);
    if (req.method == METHOD_NOT_ALLOWED_T) {
        printf("%d\n", __LINE__);
        // TODO: отправить 405
        return;
    }
    printf("%d\n", __LINE__);
    if (decode_url(req.url) != 0) {
        printf("%d\n", __LINE__);
        // TODO: отправить бед реквест
        return;
    }
    printf("%d\n", __LINE__);
    char *absolute_static_path = calloc(strlen(req.url) + strlen(document_root) + 1, sizeof(char));
    printf("%d\n", __LINE__);
    absolute_static_path = strcat(absolute_static_path, document_root);
    printf("%d\n", __LINE__);
    absolute_static_path = strcat(absolute_static_path, req.url);
    printf("%d\n", __LINE__);
    struct response_t resp = {0};
    printf("%d\n", __LINE__);
    resp.body_fd = get_static_fd(absolute_static_path);
    printf("%d\n", __LINE__);
    if (resp.body_fd <= 0) {
        printf("%d\n", __LINE__);
        if (errno == EACCES) {
            printf("%d\n", __LINE__);
            resp.status_code = 403;
        } else if (errno == ENOENT &&
                   strcmp(absolute_static_path + strlen(absolute_static_path) - strlen("index.html"), "index.html") == 0) {
            printf("%d\n", __LINE__);
            resp.status_code = 403;
        } else {
            resp.status_code = 404;
        }
        // TODO: отправить бед реквест
    }
    resp.content_length = get_content_length(resp.body_fd);
    resp.content_type = get_content_type(req.url);
    set_date(resp.date);
    resp.method = req.method;
    resp.status_code = OK_CODE;
    send_http_response(sock_fd,&resp);
    close(resp.body_fd);
// получить файловый дескриптор на статику
// установить контент лен
// установить заголовок сервер
// установить контент тайп
//    if (get_static(req.url))

// сформировать ответ
// упаковать ответ
//    send_response()
//считать запрос
//распарсить запрос
//свич-кейс по методам
//сформировать ответ
//вернуть ответ
}

