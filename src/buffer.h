#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stddef.h>

#define BUFFER_MAX_PREALLOC (1024*1024)
#define BUFFER_DEFAULT_SIZE 2048

typedef struct {
    size_t capacity;
    size_t used;
    uint8_t *data;
} buffer_t;

buffer_t *buffer_new(size_t len);
void buffer_free(buffer_t *buf);

buffer_t *buffer_reset(buffer_t *buf);
buffer_t *buffer_concat(buffer_t *buf, const char *data, size_t len);

#endif
