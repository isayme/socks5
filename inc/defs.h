#ifndef _DEFS_H
#define _DEFS_H

// define function return value macro
#define R_ERROR		-1
#undef R_OK
#define R_OK    0

// typedef some data type for global use
typedef signed char	        INT8;
typedef unsigned char       UINT8;

typedef signed short        INT16;
typedef unsigned short      UINT16;

typedef signed int          INT32;
typedef unsigned int        UINT32;

typedef signed long long    INT64;
typedef unsigned long long  UINT64;


#define  MAX(a,b)	(((a)>(b))?(a):(b))
#define  MIN(a,b)	(((a)<(b))?(a):(b))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define _perror() printf("%s : args error.\n", __func__)
#define dprintf(msg...) do{printf("%s[%d] : ", __func__, __LINE__);printf(msg);}while(0)

#define LOG(level, format, ...) \
    do { \
        fprintf(stderr, "[%s|%s@%s,%d] " format "\n", \
            level, __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0)


#endif
