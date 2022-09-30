#include <string.h>
#include <http.h>
#include <sys/socket.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>

#define MAX_REQUEST_LEN 1000
#define  METHOD_MAX_LEN 10
#define URL_MAX_LEN 990

char *document_root;
struct request_t {
    char method [METHOD_MAX_LEN];
    char url [URL_MAX_LEN];
};

static int parse_http_request(char * raw_req, struct  request_t* req) {
//    TODO: убрать хардкод 10 990
    printf("%d\n",__LINE__);
    printf("%s\n",raw_req);
    char s1 [10];
    char s2[10];

    printf("%d\n",__LINE__);
    printf("%d\n",sscanf(raw_req, "%s %s", s1,s2));
    printf("%d\n",__LINE__);
    printf("parsed request: %s, %s", s1, s2);
    printf("%d\n",__LINE__);
    if (sscanf(raw_req, "%s %s", req->method,req->url) != 2) {
        printf("%d\n",__LINE__);
        printf("parsed request: %s, %s", req->method, req->url);
        printf("%d\n",__LINE__);
        return -1;
    }
    printf("%d\n",__LINE__);
    printf("parsed request: %s, %s", req->method, req->url);
    printf("%d\n",__LINE__);
    return 0;
}

// из запроса вытащить метод -- первым словом
    // switch-case по методам
// урл / процентный урл -- вторым словом вытащить
// сформировать ответ
// отправить ответ

void init_document_root(char *new_document_root) {
    document_root = new_document_root;
}

static int read_http_request(int socket_fd, char *req, size_t max_req_len) {
    if (req == NULL || max_req_len == 0) {
        return -1;
    }
    printf("%d\n",__LINE__);
    int recvd = 0;
    int curr_recrd = 0;
    while(recvd < max_req_len && (strstr(req,"\r\n") == NULL)) {
        curr_recrd = recv(socket_fd, req + recvd, max_req_len - recvd, MSG_DONTWAIT);
        if (curr_recrd <= 0) {
            printf("%d\n",__LINE__);
            req[0] = '\0';
            return -1;
        }
        recvd += curr_recrd;
    }
    // можно занулить все после \r\n
    printf(    "    %d\n",__LINE__);
    printf("%p\n", strstr(req,"\r\n"));
    if (strstr(req,"\r\n") == NULL) {
        req[0] = '\0';
        return -1;
    }

    return 0;
}

void get_http_response_cb(int sock_fd) {
    char raw_req[MAX_REQUEST_LEN] = {0};
    if (read_http_request(sock_fd, raw_req, MAX_REQUEST_LEN) != 0) {
        err_log_code("Error reading client request", errno);
        // отправить бед реквест
        return;
    }
    printf("raw request: %s", raw_req);
    struct request_t req = {};
    if (parse_http_request(raw_req,&req) != 0) {
        printf("%d\n",__LINE__);
        err_log_code("Error parsing client request", errno);
        // отправить бед реквест
        return;
    }
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("%d\n",__LINE__);
    printf("parsed request: %s, %s", req.method, req.url);
    printf("%d\n",__LINE__);
//считать запрос
//распарсить запрос
//свич-кейс по методам
//сформировать ответ
//вернуть ответ
}

