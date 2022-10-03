#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

time_t timer;

void init_logger(void) {
    time(&timer);
}

void err_log_code(const char *err_str, int err_code) {
    fprintf(stderr, "%s: %d: %s : error code: %d; message: %s \n", ctime(&timer), getpid(), err_str, err_code, strerror(err_code));
}

void err_log(const char *err_str) {
    fprintf(stderr, "%s: %s\n", ctime(&timer), err_str);
}

void info_log(const char *info) {
    char *curr_time = ctime(&timer);
    curr_time[strlen(curr_time) - 1] = '\0';
    fprintf(stderr, "%s: %s", curr_time, info);
}
