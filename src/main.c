#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ev.h>

#include "logger.h"
#include "netutils.h"
#include "resolve.h"
#include "callback.h"

#define EPOLL_SIZE 1024
#define LISTEN_BACKLOG 128
#define MAX_EVENTS_COUNT 128
#define LISTEN_PORT 23456

static void signal_handler(int sig) {
    logger_info("receive signal: [%d]\n", sig);

    switch (sig) {
        case SIGINT:
        case SIGTERM:
            break;
        default:
            logger_warn("unkown signal [%d]\n", sig);
    }
}

static int register_signals() {
    // ctrl + c
    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        logger_error("register signal SIGINT fail\n");
        return -1;
    }

    // pkill
    if (SIG_ERR == signal(SIGTERM, signal_handler)) {
        logger_error("register signal SIGTERM fail\n");
        return -1;
    }

    return 0;
}

int create_and_bind(uint16_t port, int32_t backlog) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logger_error("sockfd create failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    set_nonblocking(sockfd);
    set_reuseaddr(sockfd);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        logger_error("bind error [%d]\n", errno);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, backlog) < 0) {
        logger_error("listen error [%d]!\n", errno);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main (int argc, char **argv) {
    logger_info("starting ...\n");

    struct ev_loop *loop = ev_default_loop(0);
    struct ev_io server;

    if (resolve_init(loop, NULL, 0) < 0) {
        logger_error("resolve_init fail\n");
        exit(EXIT_FAILURE);
    } else {
        logger_info("resolve_init ok\n");
    }

    server.fd = create_and_bind(LISTEN_PORT, LISTEN_BACKLOG);

    ev_io_init(&server, accept_cb, server.fd, EV_READ);
    ev_io_start(loop, &server);

    ev_run(loop, 0);

    logger_info("exiting ...\n");

    resolve_shutdown(loop);
    ev_loop_destroy(loop);

    return EXIT_SUCCESS;
}
