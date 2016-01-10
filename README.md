# About #
A simple socks5 protocol implementation using socket and libev.

More information about socks5 : [RFC1928](http://www.ietf.org/rfc/rfc1928.txt "RFC1928")

# How to use #
After clone the rep or download the compressed file, change directory to the project, execute command `make` to compile and build.

Execute `./bin/socks5 -h` get help infomation of the process.   
A `-p port` argument will custom listen port of the server.

# Todo #
Socks5 is a proxy of plaintext type, for more security, I'll encrypt the data transfered between local computer and the server, a model like [shadowsocks](https://github.com/clowwindy/shadowsocks "shadowsocks").

# Contact #
Email : isaymeorg [at] gmail [dot] com  
Blog  : [www.isayme.org](www.isayme.org "www.isayme.org") [Chinese Simplified]
