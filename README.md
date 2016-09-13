# ping

## Usage
Usage: ping [-h help] [-m=udp/tcp/icmp protocol] [-t=<timeout in ms>]
            [-p=<port> port for tcp/udp echo] [-s=<payload size>]
            [-c=<count> number of ping] [-i=<interval between ping in s>] destination
*NB: running an icmp ping must be done as root*


## How to install it
### Requirements
- gcc
- make
Compile in Debian 8.5
### Compile it
```
make && cp ping /usr/local/bin/pingtl

