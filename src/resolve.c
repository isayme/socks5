#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ev.h>
#include "resolve.h"
#include "netutils.h"
#include "logger.h"

static struct dns_ctx *g_ctx = &dns_defctx;
static struct ev_io g_resolve_io_watcher;
static struct ev_timer g_resolve_timeout_watcher;

static void resolve_sock_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct dns_ctx *ctx = (struct dns_ctx *)w->data;

    logger_debug("resolve_sock_cb %x\n", revents);

    if (revents & EV_READ) {
        dns_ioevent(ctx, ev_now(loop));
    }
}

static void resolve_timeout_cb(struct ev_loop *loop, struct ev_timer *w, int revents) {
    struct dns_ctx *ctx = (struct dns_ctx *)w->data;

    logger_debug("resolve_timeout_cb %x\n", revents);

    if (revents & EV_TIMER) {
        int next = dns_timeouts(NULL, -1, ev_now(loop));

        if (next > 0) {
            w->repeat = next;
            ev_timer_again (EV_A_ w);
        } else {
            w->repeat = 1;
            ev_timer_again (EV_A_ w);
        }
    }
}

static void dns_timer_setup_cb(struct dns_ctx *ctx, int timeout, void *data) {
    struct ev_loop *loop = (struct ev_loop *)data;

    logger_debug("dns_timer_setup_cb %d\n", timeout);

    if (ev_is_active(&g_resolve_timeout_watcher)) {
        ev_timer_stop(loop, &g_resolve_timeout_watcher);
    }

    if (ctx != NULL && timeout >= 0) {
        ev_timer_set(&g_resolve_timeout_watcher, timeout, 0.0);
        ev_timer_start(loop, &g_resolve_timeout_watcher);
    }
}

int resolve_init(struct ev_loop *loop, char **nameservers, int nameserver_num) {
    if (NULL == nameservers) {
        dns_reset(g_ctx);
        dns_init(g_ctx, 0);
    } else {
        dns_reset(g_ctx);

        for (int i = 0; i < nameserver_num; i++) {
            dns_add_serv(g_ctx, nameservers[i]);
        }
    }

    int sockfd = dns_open(g_ctx);
    if (sockfd < 0) {
        logger_error("dns_open fail, errno: [%d]\n", errno);
        return -1;
    }

    if (set_nonblocking(sockfd) < 0) {
        logger_error("set_nonblocking, errno: [%d]\n", errno);
        close(sockfd);
        return -1;
    }

    ev_io_init(&g_resolve_io_watcher, resolve_sock_cb, sockfd, EV_READ);
    g_resolve_io_watcher.data = g_ctx;
    ev_io_start(loop, &g_resolve_io_watcher);

    ev_timer_init(&g_resolve_timeout_watcher, resolve_timeout_cb, 0, 0.0);
    g_resolve_timeout_watcher.data = g_ctx;
    ev_timer_start(loop, &g_resolve_timeout_watcher);

    // dns_set_tmcbck(g_ctx, dns_timer_setup_cb, loop);

    return sockfd;
}

void resolve_shutdown(struct ev_loop *loop) {
    ev_io_stop(loop, &g_resolve_io_watcher);

    if (ev_is_active(&g_resolve_timeout_watcher)) {
        ev_timer_stop(loop, &g_resolve_timeout_watcher);
    }

    dns_close(g_ctx);
}

void resolve_cancel(struct resolve_query_t *query) {
    dns_cancel(g_ctx, query->q);
    free(query);
}

static void dns_query_v4_cb(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data) {
    struct resolve_query_t *query = (struct resolve_query_t *)data;

    logger_debug("dns_query_v4_cb\n");

    if (NULL == result) {
        logger_error("dns_query_v4_cb result is NULL\n");
        return;
    }

    if (result->dnsa4_nrr > 0) {
        struct sockaddr_storage storage;
        storage.ss_family = AF_INET;

        struct sockaddr_in *addr = (struct sockaddr_in *)&storage;
        addr->sin_addr = result->dnsa4_addr[0];

        query->cb(storage, query);
    }

    free(result);
}

struct resolve_query_t *resolve_query(char *hostname, resolve_cb *cb, void *data) {
    struct resolve_query_t *query = (struct resolve_query_t *)malloc(sizeof(struct resolve_query_t));

    logger_debug("resolve_query [%s]\n", hostname);

    if (NULL == query) {
        logger_error("malloc fail, errno: [%d]\n", errno);
        return NULL;
    }

    memset(query, 0, sizeof(struct resolve_query_t));
    query->data = data;
    query->cb = cb;
    query->q = dns_submit_a4(g_ctx, hostname, 0, dns_query_v4_cb, query);
    if (NULL == query->q) {
        logger_error("dns_submit_a4 fail, errno: [%d]\n", errno);
        free(query);
        return NULL;
    }

    return query;
}
