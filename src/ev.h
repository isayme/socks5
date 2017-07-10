#ifndef EV_H
#define EV_H

#include <stdbool.h>
#include <sys/epoll.h>

struct ev_loop {
    bool active;
    int efd;
    struct epoll_event *epoll_events;
    int epoll_eventmax;
};

#define EV_READ 0x01
#define EV_WRITE 0x02

struct ev_loop *ev_default_loop();
void ev_loop_destroy (struct ev_loop *loop);
void ev_run(struct ev_loop *loop);

struct ev_void {
    void (*cb)(struct ev_loop *, void *, uint32_t);
};

struct ev_io;
typedef void (ev_io_cb)(struct ev_loop *, struct ev_io *, uint32_t);
struct ev_io {
    ev_io_cb *cb;
    bool active;
    void *data;
    int fd;
    uint32_t events;
};

int ev_io_init(struct ev_io *w, int fd, ev_io_cb *cb, uint32_t events);
void ev_io_set(struct ev_io *w, int fd, uint32_t events);
int ev_io_start(struct ev_loop *loop, struct ev_io *w);
void ev_io_stop(struct ev_loop *loop, struct ev_io *w);

struct ev_timer;
typedef void (ev_timer_cb)(struct ev_loop *, struct ev_timer *, uint32_t);
struct ev_timer {
    ev_timer_cb *cb;
    bool active;
    void *data;
    int timerfd;
    double at;
    double repeat;
};

int ev_timer_init(struct ev_timer *w, ev_timer_cb *cb, double after, double repeat);
void ev_timer_set(struct ev_timer *w, double after, double repeat);
void ev_timer_again(struct ev_loop *loop, struct ev_timer *w);
int ev_timer_start(struct ev_loop *loop, struct ev_timer *w);
void ev_timer_stop(struct ev_loop *loop, struct ev_timer *w);

#endif
