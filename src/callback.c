#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#include "callback.h"
#include "logger.h"
#include "netutils.h"

void accept_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events) {
    int fd = w->fd;

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(struct sockaddr);
        int clientfd;

        clientfd = accept(fd, (struct sockaddr *)&client, &len);
        if (clientfd == -1) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                logger_error("accept error: [%d]\n", errno);
            }
            break;
        }

        if (set_nonblocking(clientfd) < 0) {
            logger_error("set_nonblocking: [%d]\n", errno);
            close(clientfd);
            return;
        }

        struct ev_io *w = (struct ev_io *)malloc(sizeof(struct ev_io));
        ev_io_init(w, clientfd, client_recv_cb, EV_READ);
        ev_io_start(loop, w);

        logger_info("accept connection [%d](host=%s, port=%d)\n", clientfd,
            inet_ntoa(client.sin_addr),
            ntohs(client.sin_port));
    }
}

void client_recv_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events) {
    int fd = w->fd;

    size_t count;
    char buf[512];

    while (1) {
        memset(buf, 0, 512);
        count = read(fd, buf, sizeof(buf));
        if (count == -1) {
            /*
                * if errno == EAGAIN, that means we have read all
                * data. So go back to the main loop.
                */
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                logger_info("closed connection [%d], errno: [%d]\n", fd, errno);
                ev_io_stop(loop, w);
            }
            break;
        } else if (count == 0) {
            /* End of file. The remote has closed the
                connection. */
            logger_info("remote closed connection [%d]\n", fd);
            ev_io_stop(loop, w);
            break;
        } else {
            logger_info("receive from [%d]: [%s]\n", fd, buf);
        }
    }
}

void client_send_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events) {

}

void remote_recv_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events) {

}

void remote_send_cb(struct ev_loop *loop, struct ev_io *w, uint32_t events) {

}
