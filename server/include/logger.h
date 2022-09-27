#ifndef WEB_SERVER_LOGGER_H
#define WEB_SERVER_LOGGER_H


void init_logger(void);
void err_log_code(const char* err_str, int err_code);
void err_log(const char* err_str);
void info_log(const char* info);

#endif //WEB_SERVER_LOGGER_H
