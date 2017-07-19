#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "netutils.h"

int create_socket(int af) {
    int fd = socket(af, SOCK_STREAM, 0);

    if (fd < 0) {
        return -1;
    }

    if (set_nonblocking(fd) < 0) {
        close(fd);
        return -1;
    }

    if (set_reuseaddr(fd) < 0) {
        close(fd);
        return -1;
    }

    if (set_nosigpipe(fd) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int set_nonblocking(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int set_reuseaddr(int fd) {
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
}

int set_nodelay(int fd) {
    int opt = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
}

int set_nosigpipe(int fd) {
#ifdef SO_NOSIGPIPE
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt, sizeof(opt));
#endif
    return 0;
}

int strtosockaddr(const char *src, void *addrptr) {
    int ret;

    struct sockaddr_storage *storage = (struct sockaddr_storage *)addrptr;

    struct sockaddr_in addr4;
    ret = inet_pton(AF_INET, src, &(addr4.sin_addr));
    if (ret > 0) {
        storage->ss_family = AF_INET;
        struct sockaddr_in *addr = (struct sockaddr_in *)addrptr;
        memcpy(&addr->sin_addr, &addr4.sin_addr, sizeof(addr4.sin_addr));
        return ret;
    }

    struct sockaddr_in6 addr6;
    ret = inet_pton(AF_INET6, src, &(addr6.sin6_addr));
    if (ret > 0) {
        storage->ss_family = AF_INET6;
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)addrptr;
        memcpy(&addr->sin6_addr, &addr6.sin6_addr, sizeof(addr6.sin6_addr));
        return ret;
    }

    return -1;
}
