#include <string.h>
#include "buffer.h"

buffer_t *buffer_new(size_t len) {
    buffer_t *buf = (buffer_t *)malloc(sizeof(buffer_t));
    if (NULL == buf) {
        return NULL;
    }

    buf->capacity = len;
    buf->used = 0;
    buf->data = (char *)malloc(len);
    if (NULL == buf->data) {
        free(buf);
        return NULL;
    }

    return buf;
}

void buffer_free(buffer_t *buf) {
    free(buf->data);
    free(buf);
}

static buffer_t *buffer_trymakeroom(buffer_t *buf, size_t addlen) {
    size_t free = buf->capacity - buf->used;

    if (free >= addlen) {
        return buf;
    }

    size_t newcapacity = buf->used + addlen;
    if (newcapacity < BUFFER_MAX_PREALLOC) {
        newcapacity *= 2;
    } else {
        newcapacity += BUFFER_MAX_PREALLOC;
    }

    buf->data = (char *)realloc(buf->data, newcapacity);
    if (NULL == buf->data) {
        return NULL;
    }
    buf->capacity = newcapacity;

    return buf;
}

buffer_t *buffer_reset(buffer_t *buf) {
    buf->used = 0;
    return buf;
}

buffer_t *buffer_concat(buffer_t *buf, const char *data, size_t len) {
    buf = buffer_trymakeroom(buf, len);
    if (NULL == buf) {
        return NULL;
    }

    memcpy(buf->data + buf->used, data, len);
    buf->used += len;

    return buf;
}
