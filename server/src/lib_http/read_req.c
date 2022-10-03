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
    if (hex_code <= '9' && hex_code >= '0') {
        return (char) (hex_code - '0');
    }
    if (hex_code <= 'f' && hex_code >= 'a') {
        return (char) (hex_code - 'a' + 10);
    }
    if (hex_code <= 'F' && hex_code >= 'A') {
        return (char) (hex_code - 'A' + 10);
    }
    return 0;
}

int decode_url(char *url) {
    size_t url_encoded_len = strlen(url) + 1;
    char url_encoded[url_encoded_len + 1];
    strncpy(url_encoded, url, url_encoded_len);
    char *end_decoded = url;
    char *begin_encoded = url_encoded;
    char *percent_pos = strchr(url_encoded, '%');
    while (percent_pos != NULL) {
        if (percent_pos - url_encoded + 3 > url_encoded_len) {
            url[0] = '\0';
            return -1;
        }
        *percent_pos = (char) (hex_to_decimal_digit(percent_pos[1]) * (char) 16 + hex_to_decimal_digit(percent_pos[2]));
        strncpy(end_decoded, begin_encoded, percent_pos - begin_encoded + 1);
        end_decoded += percent_pos - begin_encoded + 1;
        begin_encoded = percent_pos + 3;
        percent_pos = strchr(begin_encoded, '%');
    }
    strcpy(end_decoded, begin_encoded );
    end_decoded[url_encoded_len - (begin_encoded - url)] = '\0';
    return 0;
}

int parse_http_request(char *raw_req, struct request_t *req) {
    char method[METHOD_MAX_LEN];
    char http_version[HTTP_VERSION_MAX_LEN];
    if (sscanf(raw_req, "%s %s %s\n", method, req->url, http_version) != 3) {
        return -1;
    }
    req->method = get_request_method(method);
    req->version = get_request_http_version(http_version);
    char *query_param_pos = strstr(req->url, "?");
    if (query_param_pos != NULL) {
        *query_param_pos = '\0';
    }
    return 0;
}

void read_http_request(int socket_fd, char *req, size_t max_req_len) {
    int rcvd = 0;
    for (int rvd = 1; (rvd > 0) && (req[rcvd] != '\n'); rcvd += rvd) {
        rvd = recv(socket_fd, req + rcvd, max_req_len - 1, MSG_DONTWAIT);
    }
}
