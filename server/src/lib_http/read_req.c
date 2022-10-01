#include <string.h>
#include <http.h>
#include <sys/socket.h>
#include <logger.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <http_inner.h>

static enum request_method_t get_request_method(char *method) {
    if (strcmp(method, GET) == 0) {
        return GET_T;
    }
    if (strcmp(method, HEAD) == 0) {
        return HEAD_T;
    }
    return METHOD_NOT_ALLOWED_T;
}

static enum http_version_t get_request_http_version(char *http_version) {
    if (strcmp(http_version, HTTP1_0) == 0) {
        return HTTP1_0_T;
    }
    if (strcmp(http_version, HTTP1_1) == 0) {
        return HTTP1_1_T;
    }
    if (strcmp(http_version, HTTP2_0) == 0) {
        return HTTP2_0_T;
    }
    return UNKNOWN_HTTP_T;
}

static char hex_to_decimal_digit(char hex_code) {
    if (hex_code<='9' && hex_code>= '0') {
        return hex_code - '0';
    }
    if (hex_code<='f' && hex_code>= 'a') {
        return hex_code - 'a' + 10;
    }
    if (hex_code<='F' && hex_code>= 'A') {
        return hex_code - 'A' + 10;
    }
    return 0;
}

int decode_url(char *url) {
    char *end_decoded = url; // конец проверенного урла
    char *begin_encoded = url; // начало непроверенного урла
    size_t url_len = strlen(url);
    char *percent_pos = strchr(url, '%');
    while (percent_pos != NULL) {
        if (percent_pos - url + 3 > url_len) {
            url[0] = '\0';
            return -1;
        }
        percent_pos[0] = hex_to_decimal_digit(percent_pos[1] )*16 + hex_to_decimal_digit(percent_pos[2]);
        strncpy(end_decoded, begin_encoded, percent_pos - begin_encoded + 1);
        end_decoded += percent_pos - begin_encoded + 1;
        begin_encoded = percent_pos + 3;
        percent_pos = strchr(begin_encoded, '%');
    }
    strncpy(end_decoded, begin_encoded, url_len - (begin_encoded - url));
    end_decoded[url_len - (begin_encoded - url)] = '\0';
    return 0;
}


int parse_http_request(char *raw_req, struct request_t *req) {
    char method[METHOD_MAX_LEN];
    printf("%d\n",__LINE__);
    char http_version[HTTP_VERSION_MAX_LEN];
    printf("%d\n",__LINE__);
    if (sscanf(raw_req, "%s %s %s\n", method, req->url, http_version) != 3) {
        printf("%d\n",__LINE__);
        return -1;
    }
    printf("%d\n",__LINE__);
    req->method = get_request_method(method);
    printf("%d\n",__LINE__);
    req->version = get_request_http_version(http_version);
    printf("%d\n",__LINE__);
    return 0;
}

int read_http_request(int socket_fd, char *req, size_t max_req_len) {
    int recvd = 0;
    int curr_recrd = 0;
    while (recvd < max_req_len && (strstr(req, "\r\n") == NULL)) {
        curr_recrd = recv(socket_fd, req + recvd, max_req_len - recvd, MSG_DONTWAIT);
        if (curr_recrd <= 0) {
            req[0] = '\0';
            return -1;
        }
        recvd += curr_recrd;
    }
    if (strstr(req, "\r\n") == NULL) {
        req[0] = '\0';
        return -1;
    }

    return 0;
}
