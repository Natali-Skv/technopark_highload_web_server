#ifndef WEB_SERVER_SERVER_H
#define WEB_SERVER_SERVER_H

#define DEFAULT_CPU_LIMIT (sysconf(_SC_NPROCESSORS_ONLN))
#define PORT 80
#define DEFAULT_DOCUMENT_ROOT "/home/ns/tp/hl/tests_for_web_server/"
#define CLIENT_SOCKET_SEND_TIMEOUT 10
#define CLIENT_SOCKET_RECV_TIMEOUT 10

#define MAX_LEN_ROOT_PATH 100
#define MAX_LEN_SERVER_LOG_PATH 100
#define MAX_LEN_ACCESS_LOG_PATH 100
#define MAX_LOG_MSG_LEN 256

#define MAX_REQUEST_LEN 1000
#define MAX_LEN_METHOD 10
#define HTTP_VERSION_MAX_LEN 10
#define MAX_LEN_URL 990
#define DEFAULT_FILENAME "index.html"
#define MAX_LEN_ABSOLUTE_FILEPATH (MAX_LEN_ROOT_PATH + MAX_LEN_URL + strlen(DEFAULT_FILENAME))
#define MAX_LEN_DATE 40
#define MAX_LEN_HEADERS_VALUES 150

#endif //WEB_SERVER_SERVER_H

