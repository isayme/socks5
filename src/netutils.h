#ifndef NETUTILS_H
#define NETUTILS_H

int create_v4_socket();
int create_v6_socket();
int set_nonblocking(int fd);
int set_reuseaddr(int fd);
int set_nodelay(int fd);
int set_nosigpipe(int fd);
// int set_linger(int fd);
// int set_nosigpipe(int fd);

int strtosockaddr(const char *src, void *addrptr);

#endif
