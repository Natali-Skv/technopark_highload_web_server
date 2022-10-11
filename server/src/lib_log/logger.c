#include <errno.h>
#include <logger.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <server.h>

time_t timer;
FILE *server_log_file = NULL;
FILE *access_log_file = NULL;

// сделать логер инлайн, с определением уровня логирования через макросы
// считывать пути для логирования из опций запуска
// error_log & info_log acess_log ???

int init_logger(const char *server_log, const char *access_log) {
  time(&timer);
  if (!server_log || !server_log[0]) {
    server_log_file = stderr;
  } else {
    server_log_file = fopen(server_log, "w");
    if (server_log_file == NULL) {
      return OPENING_SERVER_LOG_FILE_ERR;
    }
  }
  if (!access_log || !access_log[0]) {
    access_log_file = stdout;
  } else {
    access_log_file = fopen(access_log, "w");
    if (access_log_file == NULL) {
      if (server_log_file != stderr) {
        fclose(server_log_file);
      }
      return OPENING_ACCESS_LOG_FILE_ERR;
    }
  }
  return OK;
}

int destruct_logger() {
  if (!fclose(server_log_file)) {
    return CLOSING_SERVER_LOG_FILE_ERR;
  }
  if (!fclose(access_log_file)) {
    return CLOSING_ACCESS_LOG_FILE_ERR;
  }
  return OK;
}

void err_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char fmt_time[MAX_LOG_MSG_LEN];
  snprintf(fmt_time,MAX_LOG_MSG_LEN, "[ERROR] %s: %d: %s: %s\n", strtok(ctime(&timer), "\n"), getpid(), fmt,
          strerror(errno));
  vfprintf(server_log_file, fmt_time, args);
}

void info_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char fmt_time[MAX_LOG_MSG_LEN];
  snprintf(fmt_time, MAX_LOG_MSG_LEN, "[INFO] %s: %d: %s\n", strtok(ctime(&timer), "\n"), getpid(), fmt);
  vfprintf(access_log_file, fmt_time, args);
}
