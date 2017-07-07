#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdint.h>

/**
 * logger context
 */
typedef struct logger_ctx_s {
    FILE *fp;

#define LOGGER_LEVEL_ERROR          0 << 1
#define LOGGER_LEVEL_WARNING        1 << 1
#define LOGGER_LEVEL_INFO           2 << 1
#define LOGGER_LEVEL_DEBUG          3 << 1
#define LOGGER_LEVEL_MASK           0x03 << 1
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
int logger_init(char *filename, uint8_t level);

/**
 * Close logger
 *
 */
int logger_close();

/**
 * Record a debug level log
 */
int logger_debug(const char *format, ...);

/**
 * Record a information level log
 */
int logger_info(const char *format, ...);

/**
 * Record a warning level log
 */
int logger_warn(const char *format, ...);

/**
 * Record a error level log
 */
int logger_error(const char *format, ...);

#endif // !__LOGGER_H
