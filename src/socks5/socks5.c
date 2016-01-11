#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include <fcntl.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ev.h>

#include "defs.h"
#include "liblog.h"
#include "socks5.h"

static socks5_cfg_t g_cfg = {0};
struct ev_loop *g_loop = NULL;
struct ev_io g_io_accept;

static void help();
static int32_t check_para(int argc, char **argv);
static void signal_func(int sig);
static void signal_init();

static int32_t socks5_srv_init(uint16_t port, int32_t backlog);
static int32_t socks5_srv_exit();

static int32_t socks5_sockset(int sockfd);

static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

int main(int argc, char **argv) {
    if (-1 == check_para(argc, argv)) {
        PRINTF(LEVEL_ERROR, "check argument error.\n");
        return -1;
    }

    signal_init();

    PRINTF(LEVEL_INFORM, "socks5 starting, port: %d\n", g_cfg.port);

    g_cfg.fd = socks5_srv_init(g_cfg.port, 10);
    if (-1 == g_cfg.fd) {
        PRINTF(LEVEL_ERROR, "socks server init error.\n");
        return -1;
    }

    g_cfg.state = SOCKS5_STATE_RUNNING;

    g_loop = ev_default_loop(0);

    ev_io_init(&g_io_accept, accept_cb, g_cfg.fd, EV_READ);
    ev_io_start(g_loop, &g_io_accept);

    ev_loop(g_loop, 0);

    PRINTF(LEVEL_INFORM, "time to exit.\n");
    socks5_srv_exit();
    PRINTF(LEVEL_INFORM, "exit socket server.\n");
    return 0;
}

static void help() {
    printf("Usage: socks5 [options]\n");
    printf("Options:\n");
    printf("    -p <port>       tcp listen port\n");
    printf("    -d <Y|y>        run as a daemon if 'Y' or 'y', otherwise not\n");

    printf("    -l <level>      debug log level,range [0, 5]\n");
    printf("    -h              print help information\n");
}

static int32_t check_para(int argc, char **argv) {
    int ch;
    int32_t bdaemon = 0;

    memset(&g_cfg, 0, sizeof(g_cfg));

    g_cfg.start_time = time(NULL);
    g_cfg.port = SOCKS5_PORT;
    g_cfg.state = SOCKS5_STATE_PREPARE;

    while ((ch = getopt(argc, argv, ":d:p:l:h")) != -1) {
        switch (ch) {
            case 'd':
                if (1 == strlen(optarg) && ('Y' == optarg[0] || 'y' == optarg[0])) {
                    printf("run as a daemon.\n");
                    bdaemon = 1;
                }
                break;
            case 'p':
                g_cfg.port = atoi(optarg);
                break;
            case 'l':
                if (0 > atoi(optarg) || 5 < atoi(optarg)) {
                    printf("debug level [%s] out of range [0 - 5].\n", optarg);
                    return -1;
                }
                liblog_level(atoi(optarg));
                printf("log level [%d].\n", atoi(optarg));
                break;
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;
            case '?':
                if (isprint(optopt)) {
                   printf("unknown option '-%c'.\n", optopt);
                } else {
                   printf("unknown option character '\\x%x'.\n", optopt);
                }
                break;
            case ':':
                if (isprint(optopt)) {
                   printf("missing argment for '-%c'.\n", optopt);
                } else {
                   printf("missing argment for '\\x%x'.\n", optopt);
                }
            default:
                break;
        }
    }

    if (bdaemon) {
        daemon(1, 1);
    }

    return 0;
}

static void signal_init() {
    int sig;

    // Ctrl + C
    sig = SIGINT;
    if (SIG_ERR == signal(sig, signal_func)) {
        PRINTF(LEVEL_WARNING, "%s signal[%d] failed.\n", __func__, sig);
    }

    // kill/pkill -15
    sig = SIGTERM;
    if (SIG_ERR == signal(sig, signal_func)) {
        PRINTF(LEVEL_WARNING, "%s signal[%d] failed.\n", __func__, sig);
    }
}

// signal callback
static void signal_func(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            ev_io_stop(g_loop, &g_io_accept);
            ev_break(g_loop, EVBREAK_ALL);
            //if (NULL != g_loop) ev_unloop(g_loop, EVUNLOOP_ALL);
            g_cfg.state = SOCKS5_STATE_STOP;
            PRINTF(LEVEL_INFORM, "signal [%d], exit.\n", sig);
            exit(0);
            break;
        default:
            PRINTF(LEVEL_INFORM, "signal [%d], not supported.\n", sig);
            break;
    }
}

static int32_t socks5_srv_init(uint16_t port, int32_t backlog) {
    struct sockaddr_in serv;
    int sockfd;
    int opt;
    int flags;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        PRINTF(LEVEL_ERROR, "socket error!\n");
        return -1;
    }

    bzero((char *)&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(port);

    if (-1 == (flags = fcntl(sockfd, F_GETFL, 0))) {
        flags = 0;
    }
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    opt = 1;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (uint *)&opt, sizeof(opt))) {
        PRINTF(LEVEL_ERROR, "setsockopt SO_REUSEADDR fail.\n");
        return -1;
    }

#ifdef SO_NOSIGPIPE
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt))) {
        PRINTF(LEVEL_ERROR, "setsockopt SO_NOSIGPIPE fail.\n");
        return -1;
    }
#endif

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        PRINTF(LEVEL_ERROR, "bind error [%d]\n", errno);
        return -1;
    }

    if (listen(sockfd, backlog) < 0) {
        PRINTF(LEVEL_ERROR, "listen error!\n");
        return -1;
    }

    return sockfd;
}

static int32_t socks5_srv_exit() {
    return close(g_cfg.fd);
}

static int32_t socks5_sockset(int sockfd) {
    struct timeval tmo = {0};
    int opt = 1;

    tmo.tv_sec = 2;
    tmo.tv_usec = 0;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmo, sizeof(tmo)) \
        || -1 == setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tmo, sizeof(tmo))) {
         PRINTF(LEVEL_ERROR, "setsockopt error.\n");
         return -1;
    }

#ifdef SO_NOSIGPIPE
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
#endif

    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (uint *)&opt, sizeof(opt))) {
        PRINTF(LEVEL_ERROR, "setsockopt SO_REUSEADDR fail.\n");
        return -1;
    }

    return 0;
}

static int32_t socks5_auth(int sockfd) {
    int remote = 0;
    char buff[BUFFER_SIZE];
    struct sockaddr_in addr;
    int addr_len;
    int ret;

    socks5_sockset(sockfd);

    // VERSION and METHODS
    if (-1 == recv(sockfd, buff, 2, 0)) GOTO_ERR;
    if (SOCKS5_VERSION != ((socks5_method_req_t *)buff)->ver) GOTO_ERR;
    ret = ((socks5_method_req_t *)buff)->nmethods;
    if (-1 == recv(sockfd, buff, ret, 0)) GOTO_ERR;

    // no auth
    memcpy(buff, "\x05\x00", 2);
    if (-1 == send(sockfd, buff, 2, 0)) GOTO_ERR;

    // REQUEST and REPLY
    if (-1 == recv(sockfd, buff, 4, 0)) GOTO_ERR;

    if (SOCKS5_VERSION != ((socks5_request_t *)buff)->ver
        || SOCKS5_CMD_CONNECT != ((socks5_request_t *)buff)->cmd) {
        PRINTF(LEVEL_DEBUG, "ver : %d\tcmd = %d.\n", \
            ((socks5_request_t *)buff)->ver, ((socks5_request_t *)buff)->cmd);

        ((socks5_response_t *)buff)->ver = SOCKS5_VERSION;
        ((socks5_response_t *)buff)->cmd = SOCKS5_RESPONSE_COMMAND_NOT_SUPPORTED;
        ((socks5_response_t *)buff)->rsv = 0;

        // cmd not supported
        send(sockfd, buff, 4, 0);
        goto _err;
    }

    if (SOSKC5_ADDRTYPE_IPV4 == ((socks5_request_t *)buff)->addrtype) {
        bzero((char *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;

        if (-1 == recv(sockfd, buff, 4, 0)) GOTO_ERR;
        memcpy(&(addr.sin_addr.s_addr), buff, 4);
        if (-1 == recv(sockfd, buff, 2, 0)) GOTO_ERR;
        memcpy(&(addr.sin_port), buff, 2);

        PRINTF(LEVEL_DEBUG, "type : IP, %s:%d.\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port));
    } else if (SOSKC5_ADDRTYPE_DOMAIN == ((socks5_request_t *)buff)->addrtype) {
        struct hostent *hptr;

        bzero((char *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;

        if (-1 == recv(sockfd, buff, 1, 0)) GOTO_ERR;
        ret = buff[0];
        buff[ret] = 0;
        if (-1 == recv(sockfd, buff, ret, 0)) GOTO_ERR;
        hptr = gethostbyname(buff);
        PRINTF(LEVEL_DEBUG, "type : domain [%s].\n", buff);

        if (NULL == hptr) GOTO_ERR;
        if (AF_INET != hptr->h_addrtype) GOTO_ERR;
        if (NULL == *(hptr->h_addr_list)) GOTO_ERR;
        memcpy(&(addr.sin_addr.s_addr), *(hptr->h_addr_list), 4);

        if (-1 == recv(sockfd, buff, 2, 0)) GOTO_ERR;
        memcpy(&(addr.sin_port), buff, 2);
    } else {
        ((socks5_response_t *)buff)->ver = SOCKS5_VERSION;
        ((socks5_response_t *)buff)->cmd = SOCKS5_RESPONSE_ADDRTYPE_NOT_SUPPORTED;
        ((socks5_response_t *)buff)->rsv = 0;

        // cmd not supported
        send(sockfd, buff, 4, 0);
        GOTO_ERR;
    }

    if ((remote = socket(AF_INET, SOCK_STREAM, 0)) < 0) GOTO_ERR;
    socks5_sockset(remote);

    if (0 > connect(remote, (struct sockaddr *)&addr, sizeof(addr))) {
        PRINTF(LEVEL_ERROR, "connect error.\n");

        memcpy(buff, "\x05\x05\x00\x01\x00\x00\x00\x00\x00\x00", 10);
        send(sockfd, buff, 4, 0);

        goto _err;
    }

    addr_len = sizeof(addr);
    if (0 > getpeername(remote, (struct sockaddr *)&addr, (socklen_t *)&addr_len)) GOTO_ERR;
    // reply remote address info
    memcpy(buff, "\x05\x00\x00\x01", 4);
    memcpy(buff + 4, &(addr.sin_addr.s_addr), 4);
    memcpy(buff + 8, &(addr.sin_port), 2);
    send(sockfd, buff, 10, 0);

    PRINTF(LEVEL_DEBUG, "auth ok.\n");
    return remote;

_err:
    if (0 != remote) close(remote);
    return -1;
}

static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = 0;
    int remote_fd;

    if (EV_ERROR & revents) {
        PRINTF(LEVEL_ERROR, "error event in accept.\n");
        return;
    }

    struct ev_io *w_client = (struct ev_io*)malloc(sizeof(struct ev_io));
    struct ev_io *w_serv = (struct ev_io*)malloc(sizeof(struct ev_io));
    if (NULL == w_client || NULL == w_serv) {
        PRINTF(LEVEL_ERROR, "apply memory error.\n");

        if (w_client) free(w_client);
        if (w_serv) free(w_serv);
        return;
    }

    client_fd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        free(w_client);
        free(w_serv);
        return;
    }

    if (-1 == (remote_fd = socks5_auth(client_fd))) {
        PRINTF(LEVEL_ERROR, "auth error.\n");
        close(client_fd);
        free(w_client);
        free(w_serv);
        return;
    }

    w_client->data = w_serv;
    ev_io_init(w_client, read_cb, client_fd, EV_READ);
    ev_io_start(loop, w_client);

    w_serv->data = w_client;
    ev_io_init(w_serv, read_cb, remote_fd, EV_READ);
    ev_io_start(loop, w_serv);

    return;
}

static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    char buffer[BUFFER_SIZE];
    ssize_t read;

    if (EV_ERROR & revents) {
      PRINTF(LEVEL_ERROR, "error event in read.\n");
      return;
    }

    read = recv(watcher->fd, buffer, BUFFER_SIZE, 0);
    if (read < 0) {
        PRINTF(LEVEL_ERROR, "read error [%d].\n", errno);

        if (104 == errno) {
            PRINTF(LEVEL_DEBUG, "close %d:%d.\n", watcher->fd, ((struct ev_io *)watcher->data)->fd);
            ev_io_stop(loop, watcher);
            ev_io_stop(loop, watcher->data);
            close(watcher->fd);
            close(((struct ev_io *)watcher->data)->fd);
            free(watcher->data);
            free(watcher);
            return;
        }
    } else if (0 == read) {
        PRINTF(LEVEL_DEBUG, "close %d:%d.\n", watcher->fd, ((struct ev_io *)watcher->data)->fd);
        ev_io_stop(loop, watcher);
        ev_io_stop(loop, watcher->data);
        close(watcher->fd);
        close(((struct ev_io *)watcher->data)->fd);
        free(watcher->data);
        free(watcher);
    } else {
        send(((struct ev_io *)watcher->data)->fd, buffer, read, 0);
    }

    return;
}
