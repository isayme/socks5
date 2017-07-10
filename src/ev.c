#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "ev.h"

static struct ev_loop *g_default_loop = NULL;

struct ev_loop *ev_default_loop() {
    if (!g_default_loop) {
        g_default_loop = (struct ev_loop *)malloc(sizeof(struct ev_loop));
        if (NULL == g_default_loop) {
            return NULL;
        }

        memset(g_default_loop, 0, sizeof(struct ev_loop));

        g_default_loop->epoll_eventmax = 128;
        g_default_loop->epoll_events = calloc(g_default_loop->epoll_eventmax, sizeof(struct epoll_event));
        if (NULL == g_default_loop) {
            free(g_default_loop);
            return NULL;
        }

#ifdef EPOLL_CLOEXEC
        g_default_loop->efd = epoll_create1(EPOLL_CLOEXEC);

        if (g_default_loop->efd)
#endif
            g_default_loop->efd = epoll_create(256);

        fcntl(g_default_loop->efd, F_SETFD, FD_CLOEXEC);
    }

    return g_default_loop;
}

void ev_loop_destroy (struct ev_loop *loop) {
    loop->active = false;
    free(loop->epoll_events);
    free(loop);
    close(loop->efd);
}

void ev_run(struct ev_loop *loop) {
    int nfds, i;

    loop->active = true;

    while (loop->active) {
        nfds = epoll_wait(loop->efd, loop->epoll_events, loop->epoll_eventmax, -1);
        if (nfds < 0) {
            if (errno == EINTR) {
                return;
            } else {
                perror("epoll_wait\n");
            }
        }

        for (i = 0; i < nfds; i++) {
            struct epoll_event *ev = loop->epoll_events + i;
            struct ev_void *w = (struct ev_void *)(ev->data.ptr);

            int events = (ev->events & (EPOLLOUT | EPOLLERR | EPOLLHUP) ? EV_WRITE : 0)
                | (ev->events & (EPOLLIN  | EPOLLERR | EPOLLHUP) ? EV_READ  : 0);
            w->cb(loop, w, events);
        }
    }
}

int ev_io_init(struct ev_io *w, int fd, ev_io_cb *cb, uint32_t events) {
    ev_io_set(w, fd, events);
    w->cb = cb;
    return 0;
}

void ev_io_set(struct ev_io *w, int fd, uint32_t events) {
    w->fd = fd;
    w->events = events;
}

int ev_io_start(struct ev_loop *loop, struct ev_io *w) {
    struct epoll_event event;
    uint32_t want = w->events;

    event.data.ptr = w;
    event.events = EPOLLET | (want & EV_READ  ? EPOLLIN  : 0)
                     | (want & EV_WRITE ? EPOLLOUT : 0);

    return epoll_ctl(loop->efd, EPOLL_CTL_ADD, w->fd, &event);
}

void ev_io_stop(struct ev_loop *loop, struct ev_io *w) {
    close(w->fd);
}

int setnonblocking(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int ev_timer_init(struct ev_timer *w, ev_timer_cb *cb, double after, double repeat) {
    ev_timer_set(w, after, repeat);
    w->cb = cb;

    int timerfd = timerfd_create(CLOCK_MONOTONIC, O_CLOEXEC);
    if (timerfd < 0) {
        return -1;
    }

    if (setnonblocking(timerfd) < 0) {
        return -1;
    }

    w->timerfd = timerfd;

    struct itimerspec at;
    at.it_value.tv_sec = (int32_t)after;
    at.it_value.tv_nsec = (after - (int32_t)after) * 1000000;
    at.it_interval.tv_sec = (int32_t)repeat;
    at.it_interval.tv_nsec = (repeat - (int32_t)repeat) * 1000000;

    if (timerfd_settime(timerfd, 0, &at, NULL) < 0) {
        return -1;
    }

    return 0;
}

void ev_timer_set(struct ev_timer *w, double after, double repeat) {
    w->at = after;
    w->repeat = repeat;
}

void ev_timer_again(struct ev_loop *loop, struct ev_timer *w) {
    // if (w->active) {

    // } else if (w->repeat) {
    //     w->at = w->repeat;
    //     ev_timer_start(loop, w);
    // }
}

int ev_timer_start(struct ev_loop *loop, struct ev_timer *w) {
    struct epoll_event event;

    event.data.ptr = w;
    event.events = EPOLLET | EPOLLIN;

    if (epoll_ctl(loop->efd, EPOLL_CTL_ADD, w->timerfd, &event) < 0) {
        ev_timer_stop(loop, w);
        return -1;
    }

    return 0;
}

void ev_timer_stop(struct ev_loop *loop, struct ev_timer *w) {
    w->active = false;
    close(w->timerfd);
}
