#ifndef _SOCKS5_H
#define _SOCKS5_H

#include <ev.h>
#include "defs.h"

#define BUFFER_SIZE 1514

#define GOTO_ERR do { PRINTF(LEVEL_ERROR, "go to error [%d].\n", errno); goto _err; } while (0)

typedef void (*libev_cb)(EV_P_ struct type *w, int revents);
#pragma pack(1)

#define SOCKS5_STATE_PREPARE 0
#define SOCKS5_STATE_RUNNING 1
#define SOCKS5_STATE_STOP 2
typedef struct socks5_cfg_t {
    time_t start_time;
#define SOCKS5_PORT 1080
    UINT16 port;
}socks5_cfg_t;

#define SOCKS5_VERSION 0x05
#define SOCKS5_CMD_CONNECT 0x01
#define SOCKS5_IPV4 0x01
#define SOCKS5_DOMAIN 0x03
#define SOCKS5_CMD_NOT_SUPPORTED 0x07
#define SOCKS5_ADDR_NOT_SUPPORTED 0x08


typedef struct socks5_method_req_t
{
    UINT8 ver;
    UINT8 nmethods;
}socks5_method_req_t;

typedef struct socks5_method_res_t
{
    UINT8 ver;
    UINT8 method;
}socks5_method_res_t;

typedef struct socks5_request_t
{
    UINT8 ver;
    UINT8 cmd;
    UINT8 rsv;
    UINT8 atype;
}socks5_request_t;

typedef socks5_request_t socks5_response_t;

#pragma pack()

#endif