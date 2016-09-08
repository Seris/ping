#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdint.h>

typedef enum {
	PROTOCOL_ICMP = 'i',
	PROTOCOL_UDP  = 'u',
	PROTOCOL_TCP  = 't'
} protocol_t;

typedef struct {
	const char* ip_address;
	uint16_t port;
	protocol_t protocol;
	uint32_t payload_size;
	uint32_t timeoutMS;
	uint32_t count;
} args_t;

void parseArguments(int argc, char* argv[]);

#endif
