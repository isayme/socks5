#ifndef CALLBACK_H
#define CALLBACK_H

#include "ev.h"

void accept_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events);

void client_recv_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events);
void client_send_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events);
void remote_recv_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events);
void remote_send_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events);

#endif
