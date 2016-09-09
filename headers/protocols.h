#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <stdint.h>
#include <limits.h>

#include <time_ms.h>

#define IP_HEADER_SIZE 20

typedef struct {
	uint8_t  type;
	uint8_t  code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequence;
} __attribute((packed)) icmp_echo_t;

typedef enum {
	ICMP_ECHO = 0,
	ICMP_DESTUNREACH = 3
} icmp_type_t;

typedef enum {
	ICMP_CODE_NET = 0,
	ICMP_CODE_HOST,
	ICMP_CODE_PROTO,
	ICMP_CODE_PORT,
	ICMP_CODE_FRAG,
	ICMP_CODE_ROUTE
} icmp_code_t;

typedef enum {
	ERROR = INT_MIN,

	ERROR_RECV,
	ERROR_SEND,
	ERROR_CONNECT,
	ERROR_SOCK,
	ERROR_DATACORRUPT,
	ERROR_TIMEOUT,
	ERROR_DESTUNREACH_NET,
	ERROR_DESTUNREACH_HOST,
	ERROR_DESTUNREACH_PROTO,
	ERROR_DESTUNREACH_PORT,
	ERROR_DESTUNREACH_FRAG,
	ERROR_DESTUNREACH_ROUTE,

	NO_ERROR = 0
} proto_result_t;

typedef int (*socket_ft)(void);
typedef int (*connect_ft)(int sockd);
typedef int (*send_ft)(int sockd, void *payload, ping_time_t *start, void **data);
typedef int (*recv_ft)(int sockd, void *payload, ping_time_t *start, float *latency, void *data);

typedef struct {
	socket_ft  sock;
	connect_ft connect;
	send_ft    send;
	recv_ft    recv;
} protocol_stack_t;

int tcp_create_sock(void);
int udp_create_sock(void);
int udptcp_connect(int sockd);
int udptcp_send(int sockd, void *payload, ping_time_t *start_time, void **data);
int udptcp_recv(int sockd, void *payload, ping_time_t *start_time, float *latency, void *data);

protocol_stack_t* get_protocol_stack(void);
int execute_protocol(protocol_stack_t* stack);
void print_error(int error, int count);

#endif
