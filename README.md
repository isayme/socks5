## socks5 server

## Quick Start
```
$ git submodule update --init --recursive
$ make
$ ./ssserver
```

## Help
Call ssserver with the --help option for a list of command-line arguments.

## Testing
Tests can be run with:
```
$ make test
```
The proxy server is expected to be bound to port 23456 for the tests to work. Start it with -p 23456.

### RFCs
- [RFC1035: DOMAIN NAMES - IMPLEMENTATION AND SPECIFICATION](https://tools.ietf.org/html/rfc1035)
- [RFC1928: SOCKS Protocol Version 5](https://tools.ietf.org/html/rfc1928)
- [RFC1929: Username/Password Authentication for SOCKS V5](https://tools.ietf.org/html/rfc1929)
