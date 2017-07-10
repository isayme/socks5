#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#include "logger.h"

/**
 * global logger context
 */
static logger_ctx_t g_logger = {
    NULL,
#ifdef DEBUG
    LOGGER_LEVEL_DEBUG,
#else
    LOGGER_LEVEL_INFO,
#endif
    LOGGER_COLOR_ON
};

#define LOGGER_TIMESTAMP_LEN 20
static char *format_time(char *buf) {
    time_t now = time(NULL);

    struct tm tm;
    localtime_r(&now, &tm);

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);

    return buf;
}

/**
 * color definitions
 */
#define LOGGER_COLOR_RED        "\x1b[31m"
#define LOGGER_COLOR_GREEN      "\x1b[32m"
#define LOGGER_COLOR_YELLOW     "\x1b[33m"
#define LOGGER_COLOR_WHITE      "\x1b[37m"
#define LOGGER_COLOR_RESET      "\x1b[0m"

int logger_init(char *filename, uint8_t options) {
    g_logger.fp = NULL;
    g_logger.with_color = LOGGER_COLOR_OFF;

    g_logger.fp = fopen(filename, "a");
    if (NULL == g_logger.fp) {
        return errno;
    }

    if (1 == isatty(STDOUT_FILENO) && LOGGER_COLOR_ON == (options & LOGGER_COLOR_MASK)) {
        g_logger.with_color = LOGGER_COLOR_ON;
    }

    g_logger.level = options & LOGGER_LEVEL_MASK;

    return 0;
}

int logger_close() {
    return fclose(g_logger.fp);
}

#define logger_register_logger(name, tag, log_level, color)                                 \
int name(const char *format, ...) {                                                         \
    int nwriten = 0;                                                                        \
    char timestamp[LOGGER_TIMESTAMP_LEN];                                                   \
    va_list arg;                                                                            \
                                                                                            \
    if (log_level > g_logger.level) {                                                       \
        return 0;                                                                           \
    }                                                                                       \
                                                                                            \
    if (LOGGER_COLOR_ON == g_logger.with_color) {                                           \
        fprintf(stdout, color);                                                             \
    }                                                                                       \
                                                                                            \
    if (g_logger.fp) {                                                                      \
        fprintf(g_logger.fp, "%s [" #tag "] ", format_time(timestamp));                     \
        va_start(arg, format);                                                              \
        nwriten += vfprintf(g_logger.fp, format, arg);                                      \
        va_end(arg);                                                                        \
    }                                                                                       \
    fprintf(stdout, "%s [" #tag "] ", format_time(timestamp));                              \
    va_start(arg, format);                                                                  \
    vfprintf(stdout, format, arg);                                                          \
    va_end(arg);                                                                            \
                                                                                            \
    if (LOGGER_COLOR_ON == g_logger.with_color) {                                           \
        fprintf(stdout, LOGGER_COLOR_RESET);                                                \
        fflush(stdout);                                                                     \
    }                                                                                       \
                                                                                            \
    return nwriten;                                                                         \
}                                                                                           \

logger_register_logger(logger_debug, DEBUG, LOGGER_LEVEL_DEBUG, LOGGER_COLOR_WHITE)
logger_register_logger(logger_info, INFO, LOGGER_LEVEL_INFO, LOGGER_COLOR_GREEN)
logger_register_logger(logger_warn, WARN, LOGGER_LEVEL_WARNING, LOGGER_COLOR_YELLOW)
logger_register_logger(logger_error, ERROR, LOGGER_LEVEL_ERROR, LOGGER_COLOR_RED)
