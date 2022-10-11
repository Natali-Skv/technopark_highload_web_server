#ifndef WEB_SERVER_SET_CONFIG_H
#define WEB_SERVER_SET_CONFIG_H

#include <server.h>

#define OK 0
#define WRANG_OPTIONS (-1)
#define SHOW_USAGE_MODE (-2)

struct config_t {
    int port;
    int cpu_limit;
    char document_root[MAX_LEN_ROOT_PATH];
    char server_log_path[MAX_LEN_SERVER_LOG_PATH];
    char access_log_path[MAX_LEN_ACCESS_LOG_PATH];
};

int set_config(int argc, char *argv[], struct config_t *cfg);
void set_config_default_values(struct config_t *cfg);

#endif  // WEB_SERVER_SET_CONFIG_H