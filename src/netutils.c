#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "netutils.h"

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

// int set_nosigpipe(int fd) {
//     int opt = 1;
//     return setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt, sizeof(opt));
// }
