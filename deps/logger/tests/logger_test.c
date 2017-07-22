#include <stdio.h>
#include "logger.h"

int main() {

    logger_init("test.log", LOGGER_LEVEL_TRACE);

    printf("\n##### log with color and trace level\n");
    logger_fatal("color fatal\n");
    logger_error("color error\n");
    logger_warn("color warning\n");
    logger_info("color infomation\n");
    logger_debug("color debug\n");
    logger_trace("color trace\n");
    logger_close();

    logger_init("test.log", LOGGER_LEVEL_DEBUG | LOGGER_COLOR_OFF);

    printf("\n##### log without color and debug level\n");
    logger_fatal("color fatal\n");
    logger_error("color error\n");
    logger_warn("color warning\n");
    logger_info("color infomation\n");
    logger_debug("color debug\n");
    logger_trace("color trace\n");
    logger_close();

    logger_init("test.log", LOGGER_LEVEL_WARNING);

    printf("\n##### log with color and debug warning\n");
    logger_fatal("color fatal\n");
    logger_error("color error\n");
    logger_warn("color warning\n");
    logger_info("color infomation\n");
    logger_debug("color debug\n");
    logger_trace("color trace\n");
    logger_close();

    return 0;
}
