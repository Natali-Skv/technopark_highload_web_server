#include <http_inner.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>


static char hex_to_decimal_digit(char hex_code);
static char hex_to_decimal_char(const char hex_char[2]);
static int is_valid_req_verion(const char *http_version);
static enum rest_method_t get_request_method(const char *method);

int decode_url(char *url) {
    size_t url_encoded_len = strlen(url) + 1;
    char url_encoded[url_encoded_len + 1];
    strncpy(url_encoded, url, url_encoded_len);
    char *end_decoded = url;
    char *begin_encoded = url_encoded;
    int hex_encoded_char_len = 3;
    for (char *percent_pos = strchr(url_encoded, '%'); percent_pos != NULL; percent_pos = strchr(begin_encoded, '%')) {
        if (percent_pos - url_encoded + hex_encoded_char_len > url_encoded_len) {
            url[0] = '\0';
            return -1;
        }
        *percent_pos = hex_to_decimal_char(percent_pos);
        strncpy(end_decoded, begin_encoded, percent_pos - begin_encoded + 1);
        end_decoded += percent_pos - begin_encoded + 1;
        begin_encoded = percent_pos + hex_encoded_char_len;
    }

    strncpy(end_decoded, begin_encoded, url_encoded_len - (begin_encoded - url_encoded));
    end_decoded[url_encoded_len - (begin_encoded - url)] = '\0';
    return 0;
}

int parse_request(const char *raw_req,char *dst_url, enum rest_method_t *dst_method) {
    char method_str[MAX_LEN_METHOD];
    char http_version[HTTP_VERSION_MAX_LEN];
    if (sscanf(raw_req, "%s %s %s\n", method_str, dst_url, http_version) != 3) {
        return PARSING_REQ_ERR;
    }
    *dst_method = get_request_method(method_str);
    if (!is_valid_req_verion(http_version)) {
        return UNKNOWN_HTTP_VERSION;
    }

    char *query_param_pos = strstr(dst_url, "?");
    if (query_param_pos != NULL) {
        *query_param_pos = '\0';
    }
    return OK;
}

int read_request(int socket_fd, char *req, size_t *offset, size_t max_req_len) {
    for (ssize_t rvd = 1; (rvd > 0) && (req[*offset] != '\n'); *offset += rvd) {
        rvd = recv(socket_fd, req + *offset, max_req_len - 1, 0);
        if (rvd == -1) {
            return errno == EAGAIN ? SOCK_DOESNT_READY_FOR_READ : SOCK_ERR;
        }
    }
    return OK;
}

static enum rest_method_t get_request_method(const char *method) {
    if (strcmp(method, GET) == 0) {
        return GET_T;
    }
    if (strcmp(method, HEAD) == 0) {
        return HEAD_T;
    }
    return METHOD_NOT_ALLOWED_T;
}

static int is_valid_req_verion(const char *http_version) {
    if (strcmp(http_version, HTTP1_0) || strcmp(http_version, HTTP1_1) || strcmp(http_version, HTTP2_0)) {
        return OK;
    }
    return UNKNOWN_HTTP_VERSION;
}

static char hex_to_decimal_digit(char hex_code) {
    if (hex_code <= '9' && hex_code >= '0') {
        return (char)(hex_code - '0');
    }
    if (hex_code <= 'f' && hex_code >= 'a') {
        return (char)(hex_code - 'a' + 10);
    }
    if (hex_code <= 'F' && hex_code >= 'A') {
        return (char)(hex_code - 'A' + 10);
    }
    return 0;
}

static char hex_to_decimal_char(const char hex_char[2]) {
    return (char)(hex_to_decimal_digit(hex_char[1]) * (char)16 + hex_to_decimal_digit(hex_char[2]));
}
