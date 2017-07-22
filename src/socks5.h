#ifndef SOCKS5_H
#define SOCKS5_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <ev.h>
#include "buffer.h"
#include "resolve.h"

#pragma pack(1)

// socks5 version
#define SOCKS5_VERSION 0x05

// socks5 reserved
#define SOCKS5_RSV 0x00

// socks5 auth method
#define SOCKS5_AUTH_NOAUTH 0x00
#define SOCKS5_AUTH_USERNAMEPASSWORD 0x02
#define SOCKS5_AUTH_NOACCEPTABLE 0xff

struct socks5_method_req {
    uint8_t ver;
    uint8_t nmethods;
    uint8_t methods[0];
};

struct socks5_method_res {
    uint8_t ver;
    uint8_t method;
};

// socks5 command
#define SOCKS5_CMD_CONNECT 0x01
#define SOCKS5_CMD_BIND 0x02
#define SOCKS5_CMD_UDPASSOCIATE 0x03

// socks5 address type
#define SOCKS5_ADDRTYPE_IPV4 0x01
#define SOCKS5_ADDRTYPE_DOMAIN 0x03
#define SOCKS5_ADDRTYPE_IPV6 0x04

struct socks5_request {
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv;
    uint8_t addrtype;
};

// socks5 response status
#define SOCKS5_RESPONSE_SUCCESS 0x00
#define SOCKS5_RESPONSE_SERVER_FAILURE 0x01
#define SOCKS5_RESPONSE_CONNECTION_NOT_ALLOWED 0x02
#define SOCKS5_RESPONSE_NETWORK_UNREACHABLE 0x03
#define SOCKS5_RESPONSE_HOST_UNREACHABLE 0x04
#define SOCKS5_RESPONSE_CONNECTION_REFUSED 0x05
#define SOCKS5_RESPONSE_TTL_EXPIRED 0x06
#define SOCKS5_RESPONSE_COMMAND_NOT_SUPPORTED 0x07
#define SOCKS5_RESPONSE_ADDRTYPE_NOT_SUPPORTED 0x08

struct socks5_response {
    uint8_t ver;
    uint8_t rep;
    uint8_t rsv;
    uint8_t addrtype;
};

#define SOCKS5_AUTH_USERNAMEPASSWORD_VER 0x01

#define SOCKS5_AUTH_USERNAMEPASSWORD_MAX_LEN        256
struct socks5_userpass_req {
    uint8_t ver;
    uint8_t ulen;
    char username[SOCKS5_AUTH_USERNAMEPASSWORD_MAX_LEN];
    uint8_t plen;
    char password[SOCKS5_AUTH_USERNAMEPASSWORD_MAX_LEN];
};

#define SOCKS5_AUTH_USERNAMEPASSWORD_STATUS_OK      0x00
#define SOCKS5_AUTH_USERNAMEPASSWORD_STATUS_FAIL    0x01
struct socks5_userpass_res {
    uint8_t ver;
    uint8_t status;
};

#pragma pack()

#define SOCKS5_DEFAULT_BUFFER_SIZE 128

struct socks5_client_conn {
    int fd;
    struct ev_io *rw;   // read watcher
    struct ev_io *ww;   // write watcher
    buffer_t *input;
    buffer_t *output;
};

struct socks5_remote_conn {
    int fd;
    char hostname[DNS_MAXNAME];
    uint16_t port;
    struct resolve_query_t *query;
    struct ev_io *rw;   // read watcher
    struct ev_io *ww;   // write watcher
    buffer_t *input;
    buffer_t *output;
    uint8_t addrtype;
    buffer_t *bndaddr;
};

struct socks5_server {
    size_t ulen;
    char username[SOCKS5_AUTH_USERNAMEPASSWORD_MAX_LEN];
    size_t plen;
    char password[SOCKS5_AUTH_USERNAMEPASSWORD_MAX_LEN];
    uint16_t port;
    uint8_t auth_method;
    bool daemon;
    uint8_t log_level;
};

struct socks5_conn {
    struct socks5_server *server;
    struct socks5_client_conn client;
    struct socks5_remote_conn remote;
#define SOCKS5_CONN_STAGE_EXMETHOD          1
#define SOCKS5_CONN_STAGE_USERNAMEPASSWORD  2
#define SOCKS5_CONN_STAGE_EXHOST            3
#define SOCKS5_CONN_STAGE_DNSQUERY          4
#define SOCKS5_CONN_STAGE_CONNECTING        5
#define SOCKS5_CONN_STAGE_CONNECTED         6
#define SOCKS5_CONN_STAGE_STREAM            7
#define SOCKS5_CONN_STAGE_CLOSING           8
#define SOCKS5_CONN_STAGE_CLOSED            9
    uint8_t stage;
    uint8_t method;
    struct ev_loop *loop;
};

struct socks5_conn *socks5_conn_new();
void socks5_conn_close();

void socks5_conn_setstage(struct socks5_conn *conn, uint8_t stage);

#endif
