#ifndef RESOLVE_H
#define RESOLVE_H

#include <ev.h>
#include <udns.h>

struct resolve_query_t;

typedef void resolve_cb(struct sockaddr_storage *storage, struct resolve_query_t *query);
struct resolve_query_t {
    struct dns_query *q;
    resolve_cb *cb;
    void *data;
};

int resolve_init(struct ev_loop *loop, char **nameservers, int nameserver_num);
void resolve_shutdown(struct ev_loop *loop);
void resolve_cancel(struct resolve_query_t *query);

struct resolve_query_t *resolve_query(char *hostname, resolve_cb *cb, void *data);

#endif
