#include <stdio.h>
#include <arguments.h>

/**
 * print_help
 * I wanted to add a comment because this file was pretty empty, so here's a cow :
 _________________________________________
/ “Another thing that got forgotten was   \
| the fact that against all probability a |
| sperm whale had suddenly been called    |
| into existence several miles above the  |
| surface of an alien planet.             |
|                                         |
| And since this is not a naturally       |
| tenable position for a whale, this poor |
| innocent creature had very little time  |
| to come to terms with its identity as a |
| whale before it then had to come to     |
\ terms with not being a whale any more.  /
 -----------------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
 *
 */
void print_help(void){
	printf("Usage: ping [-h help] [-m=udp/tcp/icmp protocol] [-t=<timeout in ms>]\n");
	printf("            [-p=<port> port for tcp/udp echo] [-s=<payload size>]\n");
	printf("            [-c=<count> number of ping] [-i=<interval between ping in s>] destination\n");
}

