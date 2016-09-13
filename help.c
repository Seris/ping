#include <stdio.h>
#include <arguments.h>

void print_help(void){
	printf("Usage: ping [-h help] [-m=udp/tcp/icmp protocol] [-t=<timeout in ms>]\n");
	printf("            [-p=<port> port for tcp/udp echo] [-s=<payload size>]\n");
	printf("            [-c=<count> number of ping] [-i=<interval between ping in s>] destination\n");
}

