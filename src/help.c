#include <stdio.h>
#include "help.h"

void help() {
    printf("ssserver\n");

    printf("   --username <username>    username for auth\n");
    printf("   --password <password>    password for auth\n");
    printf("   -p, --port <port>        server port, default to 1080\n");
    printf("   -d                       run in daemon\n");
    printf("   --loglevel <level>       log levels: fatal, error, warning, info, debug, trace\n");
    printf("   -h, --help               help\n");
}
