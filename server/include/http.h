#ifndef WEB_SERVER_HTTP_H
#define WEB_SERVER_HTTP_H

void get_http_response_cb(int sock_fd);
void init_document_root(char *new_document_root);

#endif //WEB_SERVER_HTTP_H




//#ifndef WEB_SERVER_HTTP_H
//#define WEB_SERVER_HTTP_H
//
//typedef struct http_request http_request;
//typedef struct http_response http_response;
//
//typedef struct config config;
//
//void send_response(struct http_response * resp);
//
//void parse_request(char * buf, http_request * req);
//
//void http_request_free(http_request * req);
//
//void http_response_free(http_response * resp);
//
//void http_cb(int sd, char * root_path);
//
//#endif //WEB_SERVER_HTTP_H
