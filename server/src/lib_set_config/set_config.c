#include <getopt.h>
#include <set_config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int set_config(int argc, char *argv[], struct config_t *dst_cfg) {
    struct option options[] = {{"help", no_argument, NULL, 'h'},
                               {"cpu_limit", required_argument, NULL, 'c'},
                               {"document_root", required_argument, NULL, 'r'},
                               {"server-log", required_argument, NULL, 's'},
                               {"access-log", required_argument, NULL, 'a'},
                               {NULL, 0, NULL, 0}};
    int opt, opt_idx = 0;
    while ((opt = getopt_long(argc, argv, "-c:r:s:a:", options, &opt_idx)) != -1) {
        switch (opt) {
            case 1: {
                printf("non-option argument %s\n", optarg);
                return WRANG_OPTIONS;
            }
            case 'c': {
                dst_cfg->cpu_limit = atoi(optarg);
                if (dst_cfg->cpu_limit <= 0) {
                    printf("cpu limit must be positive number\n");
                    return WRANG_OPTIONS;
                }
                break;
            }
            case 'r': {
                if (optarg == NULL) {
                    printf("root document path must have [1..%d] symbols\n", MAX_LEN_ROOT_PATH);
                    return WRANG_OPTIONS;
                }
                unsigned long optarg_len = strlen(optarg);
                if (optarg_len > MAX_LEN_ROOT_PATH || optarg_len <= 0) {
                    printf("root document path must have [1..%d] symbols\n", MAX_LEN_ROOT_PATH);
                    return WRANG_OPTIONS;
                }
                strncpy(dst_cfg->document_root, optarg, MAX_LEN_ROOT_PATH);
                break;
            }
            case 's': {
                if (!optarg) {
                    printf("server log path must have [1..%d] symbols\n", MAX_LEN_SERVER_LOG_PATH);
                    return WRANG_OPTIONS;
                }
                unsigned long optarg_len = strlen(optarg);
                if (optarg_len > MAX_LEN_SERVER_LOG_PATH || optarg_len == 0) {
                    printf("server log path must have [1..%d] symbols\n", MAX_LEN_SERVER_LOG_PATH);
                    return WRANG_OPTIONS;
                }
                strncpy(dst_cfg->server_log_path, optarg, MAX_LEN_SERVER_LOG_PATH);
                break;
            }
            case 'a': {
                if (!optarg) {
                    printf("access log path must have [1..%d] symbols\n", MAX_LEN_ACCESS_LOG_PATH);
                    return WRANG_OPTIONS;
                }
                unsigned long optarg_len = strlen(optarg);
                if (optarg_len > MAX_LEN_ACCESS_LOG_PATH || optarg_len == 0) {
                    printf("access log path must have [1..%d] symbols\n", MAX_LEN_ACCESS_LOG_PATH);
                    return WRANG_OPTIONS;
                }
                strncpy(dst_cfg->access_log_path, optarg, MAX_LEN_ACCESS_LOG_PATH);
                break;
            }
            case 'h': {
                printf(
                    "Usage: %s [--help|--document_root <path>|--cpu_limit <num>] [-r "
                    "<path>|-c <num>]\n",
                    argv[0]);
                return SHOW_USAGE_MODE;
            }
            case '?': {
                return WRANG_OPTIONS;
            }
            case ':': {
                return WRANG_OPTIONS;
            }
            default:
                break;
        }
    }

    return OK;
}

void set_config_default_values(struct config_t *cfg) {
    if (cfg->cpu_limit <= 0) {
        cfg->cpu_limit = DEFAULT_CPU_LIMIT;
    }
    if (!cfg->document_root[0]) {
        strcpy(cfg->document_root, DEFAULT_DOCUMENT_ROOT);
    }
    if (cfg->port <= 0) {
        cfg->port = PORT;
    }
}
