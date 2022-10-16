#ifndef WEB_SERVER_LOGGER_H
#define WEB_SERVER_LOGGER_H

#include <sys/time.h>
#include <http.h>
#define OK 0
#define OPENING_SERVER_LOG_FILE_ERR (-1)
#define OPENING_ACCESS_LOG_FILE_ERR (-2)
#define CLOSING_SERVER_LOG_FILE_ERR (-3)
#define CLOSING_ACCESS_LOG_FILE_ERR (-4)

int init_logger(const char *server_log, const char *access_log);
void err_log(const char *fmt, ...);
void info_log(const char *fmt, ...);
void access_log(const struct request_t *req);
void request_err_log(unsigned int req_id, const char *fmt, ...);
int destruct_logger();

#endif  // WEB_SERVER_LOGGER_H
