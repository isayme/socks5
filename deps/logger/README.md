# logger
logger in c

# Feature
- colorful log for terminal

# APIs
See [logger.h](./logger.h)

# Example
```
logger_init("test.log", LOGGER_LEVEL_DEBUG);

printf("\n##### log with color and debug level\n");
logger_error("color error\n");
logger_warn("color warning\n");
logger_info("color infomation\n");
logger_debug("color debug\n");

logger_close();
```

# Refers
- [When to use the different log levels?](https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels)
