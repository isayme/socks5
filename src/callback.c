#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#include "callback.h"
#include "logger.h"
#include "netutils.h"
#include "resolve.h"
#include "socks5.h"

void accept_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    int fd = w->fd;
    struct socks5_server *server = (struct socks5_server *)w->data;

    while (1) {
        struct socks5_conn *conn = NULL;
        struct sockaddr_storage storage;
        socklen_t len = sizeof(struct sockaddr_storage);
        int clientfd;

        clientfd = accept(fd, (struct sockaddr *)&storage, &len);
        if (clientfd == -1) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                logger_error("accept error: [%d]\n", errno);
            }
            break;
        }

        if (set_nonblocking(clientfd) < 0) {
            logger_error("set_nonblocking: [%d]\n", errno);
            goto _close_conn;
        }

        if (set_nosigpipe(clientfd) < 0) {
            logger_error("set_nosigpipe: [%d]\n", errno);
            goto _close_conn;
        }

        conn = socks5_conn_new();
        if (NULL == conn) {
            logger_error("socks5_conn_new fail: [%d]\n", errno);
            goto _close_conn;
        }

        conn->loop = loop;
        conn->server = server;
        conn->client.fd = clientfd;
        ev_io_init(conn->client.rw, client_recv_cb, clientfd, EV_READ);
        ev_io_init(conn->client.ww, client_send_cb, clientfd, EV_WRITE);
        // start receive handleshake
        ev_io_start(loop, conn->client.rw);

        if (AF_INET == storage.ss_family) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *addr = (struct sockaddr_in *)&storage;
            logger_info("accept connection (host=%s, port=%d)\n",
                inet_ntop(storage.ss_family, &addr->sin_addr.s_addr, ip, sizeof(ip)),
                ntohs(addr->sin_port));
        } else if (AF_INET6 == storage.ss_family) {
            char ip[INET6_ADDRSTRLEN];
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
            logger_info("accept connection (host=%s, port=%d)\n",
                inet_ntop(storage.ss_family, &addr->sin6_addr, ip, sizeof(ip)),
                ntohs(addr->sin6_port));
        }

        continue;

_close_conn:
        if (NULL != conn) {
            socks5_conn_close(conn);
        } else if (clientfd) {
            close(clientfd);
        }
    }
}

int connect_to_remote(struct socks5_conn *conn, struct sockaddr_storage *storage) {
    struct socks5_remote_conn *remote = &conn->remote;

    int fd = 0;
    socklen_t address_len;
    // use INET6_ADDRSTRLEN instead of INET_ADDRSTRLEN
    char ipaddr[INET6_ADDRSTRLEN];

    fd = create_socket(storage->ss_family);
    if (fd < 0) {
        logger_debug("create_socket fail, errno: [%d]\n", errno);
        goto _err;
    }

    if (AF_INET == storage->ss_family) {
        struct sockaddr_in *addr = (struct sockaddr_in *)storage;
        address_len = sizeof(struct sockaddr_in);
        if (NULL == inet_ntop(storage->ss_family, &(addr->sin_addr), ipaddr, sizeof(ipaddr))) {
            logger_error("inet_ntop fail, errno: [%d]\n", errno);
            goto _err;
        }
        addr->sin_port = htons(remote->port);
    } else if (AF_INET6 == storage->ss_family) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)storage;
        address_len = sizeof(struct sockaddr_in6);
        if (NULL == inet_ntop(storage->ss_family, &(addr->sin6_addr), ipaddr, sizeof(ipaddr))) {
            logger_error("inet_ntop fail, errno: [%d]\n", errno);
            goto _err;
        }
        addr->sin6_port = htons(remote->port);
    } else {
        logger_warn("invalid sa_family: [%d]\n", storage->ss_family);
        goto _err;
    }

    if ('\0' == remote->hostname[0]) {
        memcpy(remote->hostname, ipaddr, sizeof(ipaddr));
        logger_info("connecting to remote host=%s, port=%d\n", remote->hostname, remote->port);
    } else {
        logger_info("connecting to remote host=%s(%s), port=%d\n", remote->hostname, ipaddr, remote->port);
    }

    if (connect(fd, (struct sockaddr *)storage, address_len) < 0) {
        if (EINPROGRESS != errno) {
            logger_debug("connect fail, errno: [%d]\n", errno);
            goto _err;
        }
    }

    return fd;
_err:
    if (fd > 0) {
        close(fd);
    }

    return -1;
}

void dns_resolve_cb(struct sockaddr_storage *storage, struct resolve_query_t *query) {
    struct socks5_conn *conn = (struct socks5_conn *)query->data;
    struct ev_loop *loop = conn->loop;
    struct socks5_remote_conn *remote = &conn->remote;
    struct socks5_client_conn *client = &conn->client;

    struct socks5_response reply = {
        SOCKS5_VERSION,
        SOCKS5_RESPONSE_SERVER_FAILURE,
        SOCKS5_RSV,
        SOCKS5_ADDRTYPE_IPV4
    };

    if (NULL == storage) {
        goto _err;
    }

    int remotefd = connect_to_remote(conn, storage);
    if (remotefd < 0) {
        logger_error("connect_to_remote fail, errno [%d]\n", errno);
        goto _err;
    }

    remote->fd = remotefd;
    socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CONNECTING);
    ev_io_init(remote->rw, remote_recv_cb, remote->fd, EV_READ);
    ev_io_init(remote->ww, remote_send_cb, remote->fd, EV_WRITE);
    ev_io_start(loop, remote->ww);
    return;

_err:
    buffer_concat(client->output, (char *)&reply, sizeof(reply));
    socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CLOSING);
    ev_io_start(loop, client->ww);
}

void client_recv_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct socks5_conn *conn = (struct socks5_conn *)w->data;
    struct socks5_server *server = conn->server;
    struct socks5_client_conn *client = &conn->client;
    struct socks5_remote_conn *remote = &conn->remote;
    int fd = w->fd;

    logger_debug("client_recv_cb start, fd: [%d], stage: [%d]\n", fd, conn->stage);

    char buffer[512];
    ssize_t size;
    while (1) {
        size = read(fd, buffer, sizeof(buffer));
        if (size < 0) {
            if (EAGAIN != errno && EWOULDBLOCK != errno) {
                logger_info("close connection [%d], errno: [%d]\n", fd, errno);
                goto _close_conn;
            }
            break;
        } else if (0 == size) {
            logger_debug("closed connection [%d]\n", fd);
            goto _close_conn;
        } else {
            buffer_concat(conn->client.input, buffer, size);
        }
    }

    logger_debug("client_recv_cb end, fd: [%d], stage: [%d]\n", fd, conn->stage);

    switch (conn->stage) {
        case SOCKS5_CONN_STAGE_EXMETHOD: {
            struct socks5_method_req *method_req;
            method_req = (struct socks5_method_req *)client->input->data;
            // verify version
            if (SOCKS5_VERSION != method_req->ver) {
                logger_debug("invalid socks5 version: [%d]\n", method_req->ver);
                goto _close_conn;
            }
            if (buffer_len(client->input) < (method_req->nmethods + 2)) {
                logger_debug("need more data\n");
                // wating more data
                return;
            }

            struct socks5_method_res reply = {
                SOCKS5_VERSION,
                SOCKS5_AUTH_NOACCEPTABLE
            };

            int i;
            for (i = 0; i < method_req->nmethods; i++) {
                logger_debug("auth methods: [%d]\n", method_req->methods[i]);
                if (server->auth_method == method_req->methods[i]) {
                    reply.method = server->auth_method;
                    conn->method = reply.method;
                }
            }

            logger_debug("auth method: [%d]\n", reply.method);

            buffer_concat(client->output, (char *)&reply, sizeof(reply));
            if (SOCKS5_AUTH_NOACCEPTABLE == reply.method) {
                socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CLOSING);
            }

            // reset recv buffer
            buffer_reset(client->input);
            ev_io_stop(loop, w);
            // send method response
            ev_io_start(loop, client->ww);
            break;
        }
        case SOCKS5_CONN_STAGE_USERNAMEPASSWORD: {
            struct socks5_userpass_req req;
            memset(&req, 0, sizeof(struct socks5_userpass_req));

            req.ver = *client->input->data;
            if (SOCKS5_AUTH_USERNAMEPASSWORD_VER != req.ver) {
                logger_debug("invalid socks5 version: [%d]\n", *client->input->data);
                goto _close_conn;
            }

            if (buffer_len(client->input) < 2) {
                logger_warn("no username len, need more data\n");
                // wating more data
                return;
            }
            req.ulen = *(client->input->data + 1);
            if (buffer_len(client->input) < (2 + req.ulen)) {
                logger_warn("no username, need more data\n");
                // wating more data
                return;
            }
            memcpy(req.username, client->input->data + 2, req.ulen);

            if (buffer_len(client->input) < (req.ulen + 3)) {
                logger_warn("no password len, need more data\n");
                // wating more data
                return;
            }
            req.plen = *(client->input->data + req.ulen + 2);
            if (buffer_len(client->input) < (req.ulen + req.plen + 3)) {
                logger_warn("no password, need more data\n");
                // wating more data
                return;
            }
            memcpy(req.password, client->input->data + req.ulen + 3, req.plen);

            logger_debug("username/password: [%s]/[%s]\n", req.username, req.password);

            struct socks5_userpass_res res = {
                SOCKS5_AUTH_USERNAMEPASSWORD_VER,
                SOCKS5_AUTH_USERNAMEPASSWORD_STATUS_FAIL
            };

            if (server->ulen == req.ulen &&
                server->plen == req.plen &&
                0 == memcmp(&server->username, &req.username, req.ulen) &&
                0 == memcmp(&server->password, &req.password, req.ulen)) {
                res.status = SOCKS5_AUTH_USERNAMEPASSWORD_STATUS_OK;
            }

            if (SOCKS5_AUTH_USERNAMEPASSWORD_STATUS_FAIL == res.status) {
                socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CONNECTING);
            }

            buffer_concat(client->output, (char *)&res, sizeof(struct socks5_userpass_res));

            buffer_reset(client->input);
            ev_io_stop(loop, w);
            ev_io_start(loop, client->ww);
            break;
        }
        case SOCKS5_CONN_STAGE_EXHOST: {
            struct socks5_request *req = (struct socks5_request *)client->input->data;
            // verify version
            if (SOCKS5_VERSION != req->ver) {
                goto _close_conn;
            }

            // wait more data
            if (buffer_len(client->input) < sizeof(struct socks5_request)) {
                return;
            }

            struct socks5_response reply = {
                SOCKS5_VERSION,
                SOCKS5_RESPONSE_SUCCESS,
                SOCKS5_RSV,
                SOCKS5_ADDRTYPE_IPV4
            };

            if (SOCKS5_CMD_CONNECT != req->cmd) {
                logger_warn("not supported cmd: [%d]\n", req->cmd);
                reply.rep = SOCKS5_RESPONSE_COMMAND_NOT_SUPPORTED;
                goto _response_fail;
            }

            remote->addrtype = req->addrtype;
            struct sockaddr_storage storage;
            memset(&storage, 0, sizeof(struct sockaddr_storage));

            logger_debug("addrtype [%d]\n", req->addrtype);
            switch (req->addrtype) {
                case SOCKS5_ADDRTYPE_IPV4: {
                    if (buffer_len(client->input) < (sizeof(struct socks5_request) + 6)) {
                        logger_debug("wait more data\n");
                        return;
                    }

                    struct sockaddr_in *addr = (struct sockaddr_in *)&storage;
                    addr->sin_family = AF_INET;

                    char *host = client->input->data + sizeof(struct socks5_request);
                    char *port = host + 4;
                    memcpy(&addr->sin_addr.s_addr, host, 4);
                    memcpy(&addr->sin_port, port, 2);
                    remote->port = ntohs(addr->sin_port);

                    buffer_concat(remote->bndaddr, (char *)&addr->sin_addr.s_addr, 4);
                    buffer_concat(remote->bndaddr, (char *)&addr->sin_port, 2);
                    break;
                }
                case SOCKS5_ADDRTYPE_DOMAIN: {
                    // hostname length
                    if (buffer_len(client->input) < (sizeof(struct socks5_request) + 1)) {
                        logger_debug("wait more data\n");
                        return;
                    }
                    int hostname_len = *(client->input->data + sizeof(struct socks5_request));
                    if (buffer_len(client->input) < (sizeof(struct socks5_request) + hostname_len + 3)) {
                        logger_debug("wait more data\n");
                        return;
                    }

                    memcpy(remote->hostname, client->input->data + sizeof(struct socks5_request) + 1, hostname_len);

                    char *port = client->input->data + sizeof(struct socks5_request) + 1 + hostname_len;
                    uint16_t sin_port;
                    memcpy(&sin_port, port, 2);
                    remote->port = ntohs(sin_port);

                    logger_info("remote hostname: [%s:%d]\n", remote->hostname, remote->port);

                    if (strtosockaddr(remote->hostname, (void *)&storage) > 0) {
                        if (storage.ss_family == AF_INET) {
                            remote->addrtype = SOCKS5_ADDRTYPE_IPV4;
                            struct sockaddr_in *addr = (struct sockaddr_in *)&storage;
                            addr->sin_port = htons(remote->port);

                            buffer_concat(remote->bndaddr, (char *)&addr->sin_addr.s_addr, 4);
                            buffer_concat(remote->bndaddr, (char *)&addr->sin_port, 2);
                        } else if (storage.ss_family == AF_INET6) {
                            remote->addrtype = SOCKS5_ADDRTYPE_IPV6;
                            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
                            addr->sin6_port = htons(remote->port);
                            buffer_concat(remote->bndaddr, (char *)&addr->sin6_addr, 16);
                            buffer_concat(remote->bndaddr, (char *)&addr->sin6_port, 2);
                        }
                        break;
                    } else {
                        buffer_concat(remote->bndaddr, client->input->data + sizeof(struct socks5_request), hostname_len + 3);
                        remote->query = resolve_query(remote->hostname, dns_resolve_cb, conn);
                        if (NULL == remote->query) {
                            logger_error("resolve_query fail\n");
                            reply.rep = SOCKS5_RESPONSE_SERVER_FAILURE;
                            goto _response_fail;
                        }

                        socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_DNSQUERY);
                        buffer_reset(client->input);
                        ev_io_stop(loop, w);
                        return;
                    }
                    break;
                }
                case SOCKS5_ADDRTYPE_IPV6: {
                    if (buffer_len(client->input) < (sizeof(struct socks5_request) + 18)) {
                        logger_debug("wait more data\n");
                        return;
                    }

                    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
                    addr->sin6_family = AF_INET6;

                    char *host = client->input->data + sizeof(struct socks5_request);
                    char *port = host + 16;
                    memcpy(&addr->sin6_addr, host, 16);
                    memcpy(&addr->sin6_port, port, 2);
                    remote->port = ntohs(addr->sin6_port);

                    buffer_concat(remote->bndaddr, (char *)&addr->sin6_addr, 16);
                    buffer_concat(remote->bndaddr, (char *)&addr->sin6_port, 2);
                    break;
                }
                default:
                    logger_warn("not supported addrtype: [%d]\n", req->addrtype);
                    reply.rep = SOCKS5_RESPONSE_ADDRTYPE_NOT_SUPPORTED;
                    goto _response_fail;
            }

            ev_io_stop(loop, w);
            buffer_reset(client->input);

            int remotefd = connect_to_remote(conn, &storage);
            if (remotefd < 0) {
                logger_error("connect_to_remote fail, errno [%d]\n", errno);
                goto _response_fail;
            }

            remote->fd = remotefd;
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CONNECTING);
            ev_io_init(remote->rw, remote_recv_cb, remote->fd, EV_READ);
            ev_io_init(remote->ww, remote_send_cb, remote->fd, EV_WRITE);
            ev_io_start(loop, remote->ww);
            break;
        _response_fail:
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CLOSING);
            reply.rep = SOCKS5_RESPONSE_SERVER_FAILURE;
            buffer_concat(conn->client.output, (char *)&reply, sizeof(reply));
            ev_io_start(loop, client->ww);
            break;
        }
        case SOCKS5_CONN_STAGE_STREAM:
            // send to remote
            buffer_concat(remote->output, (char *)client->input->data, buffer_len(client->input));
            buffer_reset(client->input);
            ev_io_start(loop, remote->ww);
            break;
        default:
            logger_warn("unexpect stage [%d]\n", conn->stage);
            goto _close_conn;
    }

    logger_debug("client_recv_cb handled, fd: [%d], stage: [%d]\n", fd, conn->stage);

    // continue
    return;
_close_conn:
    logger_debug("client_recv_cb close conn, fd: [%d], stage: [%d]\n", fd, conn->stage);
    socks5_conn_close(conn);
}

void client_send_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct socks5_conn *conn = (struct socks5_conn *)w->data;
    struct socks5_client_conn *client = &conn->client;
    struct socks5_remote_conn *remote = &conn->remote;

    int fd = w->fd;

    logger_debug("client_send_cb start, fd: [%d], stage: [%d]\n", fd, conn->stage);

    ssize_t idx = 0;
    ssize_t size;

    while (1) {
        size_t remain = buffer_len(client->output) - idx;
        if (remain <= 0) {
            // all data send, stop
            buffer_reset(client->output);
            ev_io_stop(loop, w);
            break;
        }

        size = write(fd, client->output->data + idx, remain);
        if (size < 0) {
            if (EAGAIN != errno && EWOULDBLOCK != errno) {
                logger_debug("write fail, fd [%d], errno: [%d]\n", fd, errno);
                goto _close_conn;
            }
            // send buffer full, wait new event
            logger_warn("client_send_cb output buffer full, fd: [%d]\n", fd);
            buffer_t *buf = buffer_new(remain);
            if (NULL == buf) {
                logger_error("buffer_new fail, errno: [%d]\n", errno);
                goto _close_conn;
            }
            buffer_concat(buf, client->output->data + idx, remain);
            buffer_free(client->output);
            client->output = buf;
            break;
        } else {
            idx += size;
        }
    }

    if (SOCKS5_CONN_STAGE_EXMETHOD == conn->stage) {
        // change stage after exchange method
        if (SOCKS5_AUTH_NOAUTH == conn->method) {
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_EXHOST);
        } else if (SOCKS5_AUTH_USERNAMEPASSWORD == conn->method) {
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_USERNAMEPASSWORD);
        }
        // start receive new EXHOST/USERNAMEPASSWORD request
        ev_io_start(loop, client->rw);
    } else if (SOCKS5_CONN_STAGE_USERNAMEPASSWORD == conn->stage) {
        socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_EXHOST);
        // start receive EXHOST request
        ev_io_start(loop, client->rw);
    } else if (SOCKS5_CONN_STAGE_CONNECTED == conn->stage) {
        socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_STREAM);
        // start read real data
        ev_io_start(loop, client->rw);
        ev_io_start(loop, remote->rw);
    }

    logger_debug("client_send_cb end, fd: [%d], stage: [%d]\n", fd, conn->stage);

    // closing connection ?
    if (SOCKS5_CONN_STAGE_CLOSING == conn->stage) {
        goto _close_conn;
    }
    return;

_close_conn:
    logger_debug("client_send_cb close conn, fd: [%d], stage: [%d]\n", fd, conn->stage);
    socks5_conn_close(conn);
}

void remote_recv_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct socks5_conn *conn = (struct socks5_conn *)w->data;
    struct socks5_client_conn *client = &conn->client;
    struct socks5_remote_conn *remote = &conn->remote;
    int fd = w->fd;

    logger_debug("remote_recv_cb start, fd: [%d], stage: [%d]\n", fd, conn->stage);

    ssize_t size;
    char buffer[512];

    while (1) {
        size = read(fd, buffer, sizeof(buffer));
        if (size < 0) {
            if (EAGAIN != errno && EWOULDBLOCK != errno) {
                logger_debug("close remote connection [%d], errno: [%d]\n", fd, errno);
                goto _close_conn;
            }
            break;
        } else if (0 == size) {
            // remote closed, send rest data to client and then close
            logger_debug("closed remote connection [%d]\n", fd);
            ev_io_stop(loop, w);
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CLOSING);
            break;
        } else {
            buffer_concat(remote->input, buffer, size);
        }
    }

    buffer_concat(client->output, remote->input->data, buffer_len(remote->input));
    buffer_reset(remote->input);
    ev_io_start(loop, client->ww);

    logger_debug("remote_recv_cb end, fd: [%d], stage: [%d]\n", fd, conn->stage);

    return;

_close_conn:
    logger_debug("remote_recv_cb close conn, fd: [%d], stage: [%d]\n", fd, conn->stage);
    socks5_conn_close(conn);
}

void remote_send_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct socks5_conn *conn = (struct socks5_conn *)w->data;
    struct socks5_remote_conn *remote = &conn->remote;
    struct socks5_client_conn *client = &conn->client;
    int fd = w->fd;

    // connect remote success
    if (SOCKS5_CONN_STAGE_CONNECTING == conn->stage) {
        logger_debug("remote connected, fd: [%d], stage: [%d]\n", fd, conn->stage);

        struct socks5_response reply;
        reply.ver = SOCKS5_VERSION;
        reply.rep = SOCKS5_RESPONSE_SUCCESS;
        reply.addrtype = remote->addrtype;
        int remotefd = fd;

        struct sockaddr_storage storage;
        socklen_t len = sizeof(storage);
        if (getpeername(remotefd, (struct sockaddr *)&storage, &len) < 0) {
            logger_warn("getpeername(%s:%d) fail, errno: [%d]\n", remote->hostname, remote->port, errno);
            // something wrong
            reply.rep = SOCKS5_RESPONSE_SERVER_FAILURE;
            buffer_concat(client->output, (char *)&reply, sizeof(reply));
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CLOSING);
        } else {
            // connected
            socks5_conn_setstage(conn, SOCKS5_CONN_STAGE_CONNECTED);
            buffer_concat(client->output, (char *)&reply, sizeof(reply));
            buffer_concat(client->output, remote->bndaddr->data, buffer_len(remote->bndaddr));
            logger_info("remote connected host=%s, port=%d\n", remote->hostname, remote->port);
        }

        // notify client connect result
        ev_io_start(loop, client->ww);
        ev_io_stop(loop, w);

        return;
    }

    logger_debug("remote_send_cb start, fd: [%d], stage: [%d]\n", fd, conn->stage);

    ssize_t idx = 0;
    ssize_t size;
    while (1) {
        size_t remain = buffer_len(remote->output) - idx;
        if (remain <= 0) {
            // all data send, stop
            buffer_reset(remote->output);
            ev_io_stop(loop, w);
            break;
        }

        size = write(fd, remote->output->data + idx, remain);
        if (size < 0) {
            if (EAGAIN != errno && EWOULDBLOCK != errno) {
                logger_debug("write fail, fd [%d], errno: [%d]\n", fd, errno);
                goto _close_conn;
            }
            // send buffer full, wait new event
            logger_warn("remote_send_cb output buffer full, fd: [fd]\n", fd);
            buffer_t *buf = buffer_new(remain);
            if (NULL == buf) {
                logger_error("buffer_new fail, errno: [%d]\n", errno);
                goto _close_conn;
            }
            buffer_concat(buf, remote->output->data + idx, remain);
            buffer_free(remote->output);
            remote->output = buf;
            break;
        } else {
            idx += size;
        }
    }

    logger_debug("remote_send_cb end, fd: [%d], stage: [%d]\n", fd, conn->stage);

    return;
_close_conn:
    logger_debug("remote_send_cb close conn, fd: [%d], stage: [%d]\n", fd, conn->stage);
    socks5_conn_close(conn);
}
