#ifndef _SOCKS5_H
#define _SOCKS5_H

#include <ev.h>
#include "defs.h"

#define BUFFER_SIZE 1514

#define GOTO_ERR do { \
    PRINTF(LEVEL_ERROR, "go to error [%d].\n", errno); \
    goto _err; \
} while (0)

typedef void (*libev_cb)(EV_P_ struct type *w, int revents);

#pragma pack(1)

typedef struct {
    int32_t fd;
    time_t start_time;
#define SOCKS5_PORT 1080
    uint16_t port;
#define SOCKS5_STATE_PREPARE 0
#define SOCKS5_STATE_RUNNING 1
#define SOCKS5_STATE_STOP 2
    int32_t state;
} socks5_cfg_t;

// socks5 version
#define SOCKS5_VERSION 0x05

// socks5 auth method
#define SOCKS5_AUTH_NOAUTH 0x00
#define SOCKS5_AUTH_USERNAMEPASSWORD 0x02

typedef struct {
    uint8_t ver;
    uint8_t nmethods;
    uint8_t methods[0];
} socks5_method_req_t;

typedef struct {
    uint8_t ver;
    uint8_t method;
} socks5_method_res_t;


// socks5 command
#define SOCKS5_CMD_CONNECT 0x01
#define SOCKS5_CMD_BIND 0x02
#define SOCKS5_CMD_UDPASSOCIATE 0x03

// socks5 address type
#define SOSKC5_ADDRTYPE_IPV4 0x01
#define SOSKC5_ADDRTYPE_DOMAIN 0x03
#define SOSKC5_ADDRTYPE_IPV6 0x04

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv;
    uint8_t addrtype;
} socks5_request_t;

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

typedef socks5_request_t socks5_response_t;

#pragma pack()

#endif