#include <stdio.h>
#include "minunit.h"
#include "buffer.h"

MU_TEST(buffer_new_size0) {
    buffer_t *buf = buffer_new(0);
    mu_check(buf->size == 0);
    mu_check(buf->data == NULL);
    buffer_free(buf);
}

MU_TEST(buffer_new_size5) {
    buffer_t *buf = buffer_new(5);
    mu_check(buf->size == 5);
    mu_check(buf->len == 0);
    buffer_free(buf);
}

MU_TEST(buffer_realloc_test) {
    buffer_t *buf = buffer_new(5);
    buffer_concat(buf, "hello", 5);
    mu_check(buf->size == 5);
    mu_check(buf->len == 5);

    buffer_concat(buf, "abc", 3);
    mu_check(buf->size == 16);
    mu_check(buf->len == 8);

    buffer_free(buf);
}

MU_TEST(buffer_len_test) {
    buffer_t *buf = buffer_new(0);
    buffer_concat(buf, "hello", 3);
    mu_check(buffer_len(buf) == 3);
    buffer_free(buf);
}

MU_TEST(buffer_size_test) {
    buffer_t *buf = buffer_new(4);
    mu_check(buffer_size(buf) == 4);
    buffer_free(buf);
}

MU_TEST(buffer_concat_test) {
    buffer_t *buf = buffer_new(4);
    buffer_concat(buf, "abc", 3);
    buffer_concat(buf, "defg", 4);
    int result = memcmp(buf->data, "abcdefg", 7);
    mu_check(result == 0);
    buffer_free(buf);
}

MU_TEST(buffer_slice_test) {
    buffer_t *src = buffer_new(4);
    buffer_concat(src, "abcdefg", 7);
    buffer_t *dst = buffer_slice(src, 1, 5);

    int result = memcmp(dst->data, "bcde", 4);
    mu_check(result == 0);
    mu_check(dst->size == 4);
    mu_check(dst->len == 4);
    buffer_free(src);
    buffer_free(dst);
}

MU_TEST(buffer_reset_test) {
    buffer_t *buf = buffer_new(4);
    buffer_reset(buf);
    mu_check(buf->size == 0);
    mu_check(buf->len == 0);
    mu_check(buf->data == NULL);
    buffer_free(buf);
}

int main() {
    MU_RUN_TEST(buffer_new_size0);
    MU_RUN_TEST(buffer_new_size5);

    MU_RUN_TEST(buffer_concat_test);

    MU_RUN_TEST(buffer_len_test);

    MU_RUN_TEST(buffer_size_test);

    MU_RUN_TEST(buffer_realloc_test);

    MU_RUN_TEST(buffer_slice_test);

    MU_RUN_TEST(buffer_reset_test);

    MU_REPORT();

    return 0;
}
