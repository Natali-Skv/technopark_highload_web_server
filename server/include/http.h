#ifndef WEB_SERVER_HTTP_H
#define WEB_SERVER_HTTP_H

void get_http_response_cb(int sock_fd);
void init_document_root(char *new_document_root);

#endif //WEB_SERVER_HTTP_H
