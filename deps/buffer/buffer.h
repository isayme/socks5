#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdlib.h>

#define BUFFER_MAX_PREALLOC (1024*1024)
#define BUFFER_DEFAULT_SIZE 2048

typedef struct {
    size_t size;
    size_t len;
    char *data;
} buffer_t;

buffer_t *buffer_new(size_t size);
void buffer_free(buffer_t *buf);

size_t buffer_len(buffer_t *buf);
size_t buffer_size(buffer_t *buf);

buffer_t *buffer_concat(buffer_t *buf, const char *data, size_t size);
buffer_t *buffer_slice(buffer_t *buf, size_t start, size_t end);

buffer_t *buffer_reset(buffer_t *buf);

#endif
