#include <stdio.h>
#include <time.h>

#include "benchmark.h"

char *b_strftime(char *buf) {
    time_t now = time(NULL);

    struct tm tm;
    localtime_r(&now, &tm);

    strftime(buf, 20, "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

char *b_sprinft(char *buf) {
    time_t now = time(NULL);

    struct tm tm;
    localtime_r(&now, &tm);

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

int main() {
    int i;
    int count = 1 << 16;
    char buf[20];

    double start_time, end_time;

    start_time = get_cpu_time();
    for (i = 0; i < count; i++) {
        b_sprinft(buf);
    }
    end_time = get_cpu_time();
    printf("sprinft:\t%f\n", end_time - start_time);

    start_time = get_cpu_time();
    for (i = 0; i < count; i++) {
        b_strftime(buf);
    }
    end_time = get_cpu_time();
    printf("strftime:\t%f\n", end_time - start_time);

    printf("sprinft:\t%s\n", b_sprinft(buf));
    printf("strftime:\t%s\n", b_strftime(buf));

    return 0;
}
