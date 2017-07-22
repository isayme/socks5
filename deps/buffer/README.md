## buffer
A buffer in C.

## APIs
### buffer_t *buffer_new(size_t size)
create a buffer with default `size`.

### void buffer_free(buffer_t *buf)
release memory used by `buf`

### size_t buffer_len(buffer_t *buf)
get length of `buf`

### size_t buffer_size(buffer_t *buf);
get capacity of `buf`

### buffer_t *buffer_concat(buffer_t *buf, const char *data, size_t len)
append `data` to `buf`

### buffer_t *buffer_slice(buffer_t *buf, size_t start, size_t end)
create a new buffer from `buf` with slice

### buffer_t *buffer_reset(buffer_t *buf)
reset the `buf->len`&`buf->size` to `0` and free `buf->data`
