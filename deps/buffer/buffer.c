#include <string.h>
#include "buffer.h"

buffer_t *buffer_new(size_t size) {
    buffer_t *buf = (buffer_t *)malloc(sizeof(buffer_t));
    if (NULL == buf) {
        return NULL;
    }

    buf->size = size;
    buf->len = 0;
    buf->data = NULL;

    if (size > 0) {
        buf->data = (char *)malloc(size);

        if (NULL == buf->data) {
            free(buf);
            return NULL;
        }
    }

    return buf;
}

void buffer_free(buffer_t *buf) {
    free(buf->data);
    free(buf);
}

size_t buffer_len(buffer_t *buf) {
    return buf->len;
}

size_t buffer_size(buffer_t *buf) {
    return buf->size;
}

static buffer_t *buffer_trymakeroom(buffer_t *buf, size_t addsize) {
    size_t newsize = buffer_len(buf) + addsize;
    if (buffer_size(buf) >= newsize) {
        return buf;
    }

    if (newsize < BUFFER_MAX_PREALLOC) {
        newsize *= 2;
    } else {
        newsize += BUFFER_MAX_PREALLOC;
    }

    char *data = (char *)realloc(buf->data, newsize);
    if (NULL == data) {
        return NULL;
    }

    buf->data = data;
    buf->size = newsize;

    return buf;
}

buffer_t *buffer_concat(buffer_t *buf, const char *data, size_t len) {
    buf = buffer_trymakeroom(buf, len);
    if (NULL == buf) {
        return NULL;
    }

    memcpy(buf->data + buf->len, data, len);
    buf->len += len;

    return buf;
}

buffer_t *buffer_slice(buffer_t *src, size_t start, size_t end) {
    size_t size = end - start;

    buffer_t *buf = buffer_new(size);
    if (NULL == buf) {
        return NULL;
    }

    return buffer_concat(buf, src->data + start, size);
}

buffer_t *buffer_reset(buffer_t *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->len = 0;
    return buf;
}
