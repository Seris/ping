build:
	gcc -Wall -g -D_BSD_SOURCE --std=c99 -Iheaders/ *.c protocols/*.c -o ping

