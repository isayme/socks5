#include <string.h>
#include <unistd.h>
#include <ev.h>
#include "socks5.h"
#include "callback.h"
#include "logger.h"

struct socks5_conn *socks5_conn_new() {
    struct socks5_conn *conn = (struct socks5_conn *)malloc(sizeof(struct socks5_conn));
    if (NULL == conn) {
        goto _clean;
    }

    memset(conn, 0, sizeof(struct socks5_conn));

    conn->stage = SOCKS5_CONN_STAGE_EXMETHOD;

    conn->client.input = buffer_new(SOCKS5_DEFAULT_BUFFER_SIZE);
    if (NULL == conn->client.input) {
        goto _clean;
    }
    conn->client.output = buffer_new(SOCKS5_DEFAULT_BUFFER_SIZE);
    if (NULL == conn->client.output) {
        goto _clean;
    }
    conn->client.rw = (struct ev_io *)malloc(sizeof(struct ev_io));
    if (NULL == conn->client.rw) {
        goto _clean;
    }
    conn->client.rw->data = conn;
    conn->client.ww = (struct ev_io *)malloc(sizeof(struct ev_io));
    if (NULL == conn->client.ww) {
        goto _clean;
    }
    conn->client.ww->data = conn;

    conn->remote.input = buffer_new(SOCKS5_DEFAULT_BUFFER_SIZE);
    if (NULL == conn->remote.input) {
        goto _clean;
    }
    conn->remote.output = buffer_new(SOCKS5_DEFAULT_BUFFER_SIZE);
    if (NULL == conn->remote.output) {
        goto _clean;
    }
    conn->remote.bndaddr = buffer_new(SOCKS5_DEFAULT_BUFFER_SIZE);
    if (NULL == conn->remote.bndaddr) {
        goto _clean;
    }
    conn->remote.rw = (struct ev_io *)malloc(sizeof(struct ev_io));
    if (NULL == conn->remote.rw) {
        goto _clean;
    }
    conn->remote.rw->data = conn;
    conn->remote.ww = (struct ev_io *)malloc(sizeof(struct ev_io));
    if (NULL == conn->remote.ww) {
        goto _clean;
    }
    conn->remote.ww->data = conn;

    return conn;

_clean:
    if (conn) {
        socks5_conn_close(conn);
    }
    return NULL;
}

void socks5_conn_close(struct socks5_conn *conn) {
    struct ev_loop *loop = conn->loop;

    conn->stage = SOCKS5_CONN_STAGE_CLOSED;

    if (conn->client.fd) ev_io_stop(loop, conn->client.rw);
    if (conn->client.fd) ev_io_stop(loop, conn->client.ww);
    if (conn->remote.fd) ev_io_stop(loop, conn->remote.rw);
    if (conn->remote.fd) ev_io_stop(loop, conn->remote.ww);

    if (conn->client.input) {
        buffer_free(conn->client.input);
    }
    if (conn->client.output) {
        buffer_free(conn->client.output);
    }
    if (conn->client.rw) {
        free(conn->client.rw);
    }
    if (conn->client.ww) {
        free(conn->client.ww);
    }
    if (conn->client.fd) {
        close(conn->client.fd);
    }

    if (conn->remote.input) {
        buffer_free(conn->remote.input);
    }
    if (conn->remote.output) {
        buffer_free(conn->remote.output);
    }
    if (conn->remote.bndaddr) {
        buffer_free(conn->remote.bndaddr);
    }
    if (conn->remote.rw) {
        free(conn->remote.rw);
    }
    if (conn->remote.ww) {
        free(conn->remote.ww);
    }
    if (conn->remote.fd) {
        close(conn->remote.fd);
    }
}

void socks5_conn_setstage(struct socks5_conn *conn, uint8_t stage) {
    logger_debug("change stage from [%d] to [%d]\n", conn->stage, stage);
    conn->stage = stage;
}
