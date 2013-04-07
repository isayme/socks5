#ifndef _LOG_H
#define	_LOG_H

#include "defs.h"

// prefix of log file
#define LIBLOG_PREFIX "log"

// define log levels
#define LEVEL_POS       0
#define LEVEL_LEN       3
#define LEVEL_MASK      ((UINT64)0x07UL << LEVEL_POS)

#define LEVEL_TEST      ((UINT64)0UL<<LEVEL_POS)
#define LEVEL_DEBUG     ((UINT64)1UL<<LEVEL_POS)
#define LEVEL_INFORM    ((UINT64)2UL<<LEVEL_POS)
#define LEVEL_ALARM     ((UINT64)3UL<<LEVEL_POS)
#define LEVEL_WARNING   ((UINT64)4UL<<LEVEL_POS)
#define LEVEL_ERROR     ((UINT64)5UL<<LEVEL_POS)

// define log out colors, front color 3x, background color 4x
#define COLOR_POS       61
#define COLOR_LEN       3
#define COLOR_MASK      ((UINT64)0x07UL << COLOR_POS)

#define COLOR_BLACK     ((UINT64)0UL<<COLOR_POS)
#define COLOR_RED       ((UINT64)1UL<<COLOR_POS)
#define COLOR_GREEN     ((UINT64)2UL<<COLOR_POS)
#define COLOR_YELLOW    ((UINT64)3UL<<COLOR_POS)
#define COLOR_BLUE      ((UINT64)4UL<<COLOR_POS)
#define COLOR_PURPLE    ((UINT64)5UL<<COLOR_POS)
#define COLOR_CYAN      ((UINT64)6UL<<COLOR_POS)
#define COLOR_WHITE     ((UINT64)7UL<<COLOR_POS)

// define text show mode
#define TEXT_POS        58
#define TEXT_LEN        3
#define TEXT_MASK       ((UINT64)0x07UL << TEXT_POS)

#define TEXT_DEFAULT    ((UINT64)0UL<<TEXT_POS)
#define TEXT_BRIGHT     ((UINT64)1UL<<TEXT_POS)
#define TEXT_DIM        ((UINT64)2UL<<TEXT_POS)
#define TEXT_UNDERLINE  ((UINT64)4UL<<TEXT_POS)
#define TEXT_TWINKLE    ((UINT64)5UL<<TEXT_POS)
#define TEXT_REVERSE    ((UINT64)7UL<<TEXT_POS)

// define log time
#define TIME_POS        57
#define TIME_LEN        1
#define TIME_MASK       ((UINT64)0x01UL << TIME_POS)

#define TIME_SHOW       ((UINT64)0UL<<TIME_POS)
#define TIME_HIDE       ((UINT64)1UL<<TIME_POS)


INT32 liblog_log(UINT64 mode, char *format, ...);

void liblog_range(UINT32 start, UINT32 end);
UINT32 liblog_range_start();
UINT32 liblog_range_end();
INT32 liblog_level(UINT64 level);

/*#define PRINTF(mode, format, ...) \
    do { \
        liblog_log(mode, "%06d "format, __LINE__, ##__VA_ARGS__);   \
    } while (0)*/
#define PRINTF(mode, format, ...) \
    do { \
        if (__LINE__ >= liblog_range_start() && __LINE__ <= liblog_range_end()) \
        { \
            liblog_log(mode, "{%s:%d} "format, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#endif
