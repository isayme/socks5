#ifndef _DEFS_H
#define _DEFS_H

#include <stdint.h>

#define _perror() printf("%s : args error.\n", __func__)
#define dprintf(msg...) do{printf("%s[%d] : ", __func__, __LINE__);printf(msg);}while(0)

#define LOG(level, format, ...) \
    do { \
        fprintf(stderr, "[%s|%s@%s,%d] " format "\n", \
            level, __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0)


#endif
