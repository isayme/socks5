#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdint.h>

/**
 * color definitions
 */
#define LOGGER_COLOR_RED        "\x1b[31m"
#define LOGGER_COLOR_RED_BOLD   "\x1b[31;1m"
#define LOGGER_COLOR_GREEN      "\x1b[32m"
#define LOGGER_COLOR_YELLOW     "\x1b[33m"
#define LOGGER_COLOR_PURPLE     "\x1b[35m"
#define LOGGER_COLOR_CYAN       "\x1b[36m"
#define LOGGER_COLOR_WHITE      "\x1b[37m"
#define LOGGER_COLOR_RESET      "\x1b[0m"

/**
 * logger context
 */
typedef struct logger_ctx_s {
    FILE *fp;

#define LOGGER_LEVEL_TRACE          0 << 1
#define LOGGER_LEVEL_DEBUG          1 << 1
#define LOGGER_LEVEL_INFO           2 << 1
#define LOGGER_LEVEL_WARNING        3 << 1
#define LOGGER_LEVEL_ERROR          4 << 1
#define LOGGER_LEVEL_FATAL          5 << 1
#define LOGGER_LEVEL_MASK           0x07 << 1
    uint8_t level;

#define LOGGER_COLOR_OFF            1
#define LOGGER_COLOR_ON             0
#define LOGGER_COLOR_MASK           0x01
    uint8_t with_color;
} logger_ctx_t;

/**
 * Initialize logger
 * @param filename log file path&name
 * @param options for logger
 *   LOGGER_COLOR_ON: (default) enable colorful log, only for ternimal;
 *   LOGGER_COLOR_OFF: disable colorful log;
 *   LOGGER_LEVEL_DEBUG, LOGGER_LEVEL_INFO, LOGGER_LEVEL_WARNING, LOGGER_LEVEL_ERROR: define log level
 */
int logger_init(char *filename, uint8_t options);

/**
 * Close logger
 *
 */
int logger_close();

/**
 * Record a log
 */
int logger_printf(uint8_t log_level, const char *color, const char *format, ...);

// level file_path:func_name:line_number
#define LOGGER_PREFIX "[%s] (%s:%s:%d) "

#define logger_trace(format, ...)   \
    logger_printf(LOGGER_LEVEL_TRACE, LOGGER_COLOR_CYAN, LOGGER_PREFIX format, "TRACE", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define logger_debug(format, ...)   \
    logger_printf(LOGGER_LEVEL_DEBUG, LOGGER_COLOR_WHITE, LOGGER_PREFIX format, "WARN", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define logger_info(format, ...)    \
    logger_printf(LOGGER_LEVEL_INFO, LOGGER_COLOR_GREEN, LOGGER_PREFIX format, "INFO", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define logger_warn(format, ...)    \
    logger_printf(LOGGER_LEVEL_WARNING, LOGGER_COLOR_YELLOW, LOGGER_PREFIX format, "WARN", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define logger_error(format, ...)   \
    logger_printf(LOGGER_LEVEL_ERROR, LOGGER_COLOR_RED, LOGGER_PREFIX format, "ERROR", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define logger_fatal(format, ...)   \
    logger_printf(LOGGER_LEVEL_FATAL, LOGGER_COLOR_RED_BOLD, LOGGER_PREFIX format, "FATAL", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif // !__LOGGER_H
