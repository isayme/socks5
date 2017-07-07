#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "logger.h"
#include "netutils.h"

#define EPOLL_SIZE 1024
#define LISTEN_BACKLOG 128
#define MAX_EVENTS_COUNT 128
#define LISTEN_PORT 23456

int create_and_bind(uint16_t port, int32_t backlog) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logger_error("sockfd create failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    bzero((char *)&servaddr, sizeof(servaddr));
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

    int efd; // epoll fd
    int sfd; // server socket fd
    struct epoll_event event;
    struct epoll_event *events;

    efd = epoll_create(EPOLL_SIZE);

    sfd = create_and_bind(LISTEN_PORT, LISTEN_BACKLOG);

    event.data.fd = sfd;
    event.events = EPOLLIN;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) {
        logger_error("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    events = calloc(MAX_EVENTS_COUNT, sizeof(struct epoll_event));

    for (;;) {
        int nfds, i;

        nfds = epoll_wait(efd, events, MAX_EVENTS_COUNT, -1);
        for (i = 0; i < nfds; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                /*
                * An error has occured on this fd, or the socket is not
                * ready for reading (why were we notified then?)
                */
                logger_info("socket error, close the fd\n");
                close(events[i].data.fd);
                continue;
            } else if (sfd == events[i].data.fd) {
                /*
                * We have a notification on the listening socket,
                * which means one or more incoming connections.
                */
                struct sockaddr_in client;
                socklen_t len = sizeof(struct sockaddr);
                int clientfd;

                clientfd = accept(sfd, (struct sockaddr *)&client, &len);
                if (clientfd == -1) {
                    logger_warn("accept error: [%d]\n", errno);
                    break;
                }

                if (set_nonblocking(clientfd) < 0) {
                    logger_error("set_nonblocking: [%d]\n", errno);
                    exit(EXIT_FAILURE);
                }

                event.data.fd = clientfd;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, clientfd, &event) < 0) {
                    logger_error("epoll_ctl: [%d]\n", errno);
                    exit(EXIT_FAILURE);
                }

                logger_info("accept connection [%d](host=%s, port=%d)\n", clientfd,
                    inet_ntoa(client.sin_addr),
                    ntohs(client.sin_port));
            } else {
                int fd = events[i].data.fd;
                size_t count;
                char buf[512];

                while (1) {
                    bzero(buf, 512);
                    count = read(fd, buf, sizeof(buf));
                    if (count == -1) {
                        /*
                        * if errno == EAGAIN, that means we have read all
                        * data. So go back to the main loop.
                        */
                        if (errno != EAGAIN) {
                            logger_info("closed connection [%d], errno: [%d]\n", fd, errno);
                            close(fd);
                        }
                        break;
                    } else if (count == 0) {
                        /* End of file. The remote has closed the
                            connection. */
                        logger_info("remote closed connection [%d]\n", fd);
                        close(fd);
                        break;
                    } else {
                        logger_info("receive: [%s]\n", buf);
                    }
                }
            }
        }
    }

    free(events);
    close(sfd);
    close(efd);

    return 0;
}
