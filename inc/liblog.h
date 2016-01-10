#ifndef _LOG_H
#define	_LOG_H

#include "defs.h"

// prefix of log file
#define LIBLOG_PREFIX "socks5"

// define log levels
#define LEVEL_POS       0
#define LEVEL_LEN       3
#define LEVEL_MASK      ((uint64_t)0x07UL << LEVEL_POS)

#define LEVEL_TEST      ((uint64_t)0UL<<LEVEL_POS)
#define LEVEL_DEBUG     ((uint64_t)1UL<<LEVEL_POS)
#define LEVEL_INFORM    ((uint64_t)2UL<<LEVEL_POS)
#define LEVEL_ALARM     ((uint64_t)3UL<<LEVEL_POS)
#define LEVEL_WARNING   ((uint64_t)4UL<<LEVEL_POS)
#define LEVEL_ERROR     ((uint64_t)5UL<<LEVEL_POS)

// define log out colors, front color 3x, background color 4x
#define COLOR_POS       61
#define COLOR_LEN       3
#define COLOR_MASK      ((uint64_t)0x07UL << COLOR_POS)

#define COLOR_BLACK     ((uint64_t)0UL<<COLOR_POS)
#define COLOR_RED       ((uint64_t)1UL<<COLOR_POS)
#define COLOR_GREEN     ((uint64_t)2UL<<COLOR_POS)
#define COLOR_YELLOW    ((uint64_t)3UL<<COLOR_POS)
#define COLOR_BLUE      ((uint64_t)4UL<<COLOR_POS)
#define COLOR_PURPLE    ((uint64_t)5UL<<COLOR_POS)
#define COLOR_CYAN      ((uint64_t)6UL<<COLOR_POS)
#define COLOR_WHITE     ((uint64_t)7UL<<COLOR_POS)

// define text show mode
#define TEXT_POS        58
#define TEXT_LEN        3
#define TEXT_MASK       ((uint64_t)0x07UL << TEXT_POS)

#define TEXT_DEFAULT    ((uint64_t)0UL<<TEXT_POS)
#define TEXT_BRIGHT     ((uint64_t)1UL<<TEXT_POS)
#define TEXT_DIM        ((uint64_t)2UL<<TEXT_POS)
#define TEXT_UNDERLINE  ((uint64_t)4UL<<TEXT_POS)
#define TEXT_TWINKLE    ((uint64_t)5UL<<TEXT_POS)
#define TEXT_REVERSE    ((uint64_t)7UL<<TEXT_POS)

// define log time
#define TIME_POS        57
#define TIME_LEN        1
#define TIME_MASK       ((uint64_t)0x01UL << TIME_POS)

#define TIME_SHOW       ((uint64_t)0UL<<TIME_POS)
#define TIME_HIDE       ((uint64_t)1UL<<TIME_POS)


int32_t liblog_log(uint64_t mode, char *format, ...);

void liblog_range(uint32_t start, uint32_t end);
uint32_t liblog_range_start();
uint32_t liblog_range_end();
int32_t liblog_level(uint64_t level);

#define PRINTF(mode, format, ...) \
    do { \
        if (__LINE__ >= liblog_range_start() && __LINE__ <= liblog_range_end()) { \
            liblog_log(mode, "{%s:%d} "format, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#endif
