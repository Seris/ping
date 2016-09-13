#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time_ms.h>
#include <arguments.h>
#include <protocols.h>

extern args_t arguments;

/**
 * icmp_checksum Compute the checksum of header and data of the icmp packet
 * (cf. RFC782 https://tools.ietf.org/html/rfc792 for more information on that part)
 * @params <icmp_echo_t*> header : header of the icmp packet
 * @params <uint16_t*> data : Payload of the icmp packet
 * @params <int> datalen : Length of the payload pointed by data
 * @return <uint16_t> Checksum of the icmp packet
 **/
uint16_t icmp_checksum(icmp_echo_t *header, uint16_t *data, int datalen){
	uint32_t checksum = 0;
	uint16_t *cursor;
	
	cursor = (uint16_t*) header;
	for(int i = 0; i < 4; i++){
		checksum += *cursor++;
	}

	cursor = data;
	while(datalen > 1){
		checksum += *cursor++;
		datalen -= 2;
	}

	if(datalen > 0){
		checksum += *(uint8_t*) cursor;
	}

	checksum =  (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	return (uint16_t) ~checksum;
}

/**
 * icmp_create_packet Create the ICMP packet that will be sent to the target
 * @params <icmp_echo_t*> header : The header of the packet
 * @params <void*> payload : The payload of arguments.payload_size length
 * @params <void**> buf : A pointer to the pointer that will point to the buffer
                          that will be sent. Must be free bu the user
 * @return <int> Size of the buffer
 **/
int icmp_create_packet(icmp_echo_t *header, void *payload, void **buf){
	ping_time_t time = time_ms();
	int size = arguments.payload_size + sizeof(icmp_echo_t);
	*buf = malloc(size);

	header->type = 8;
	header->code = 0;
	header->checksum = 0;
	header->identifier = getpid();
	header->sequence = (uint16_t) (time.tv_usec + time.tv_sec / 1000000);
	header->checksum = icmp_checksum(header, payload, arguments.payload_size);

	memset(*buf, 0, size);
	memcpy(*buf, header, sizeof(icmp_echo_t));
	memcpy(*buf + sizeof(icmp_echo_t), payload, arguments.payload_size);

	return size;
}

int icmp_create_sock(void){
	int sockd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockd < 0){
		return ERROR_SOCK;
	} else {
		return sockd;
	}
}

int icmp_connect(int sockd){
	return NO_ERROR;
}

int icmp_send(int sockd, void *payload, ping_time_t *start_time, void **data){
	int result = NO_ERROR;
	void *buffer;
	icmp_echo_t *header = malloc(sizeof(icmp_echo_t));
	int size = icmp_create_packet(header, payload, &buffer);
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = 0
	};
	inet_aton(arguments.ip_address, &addr.sin_addr);

	if(sendto(sockd, buffer, size, 0, (struct sockaddr*) &addr, sizeof(addr)) < 0){
		result = ERROR_SEND;
		free(header);
	} else {
		*data = header;
		*start_time = time_ms();
	}
	free(buffer);
	return result;
}

/**
 * handle_icmp_packet When receiving a packet, this function parse it, check if it's a response for our ping, an error for us etc..
 * @params <icmp_echo_t*> req : the header of the request we've done
 * @params <void*> req_pl : the payload we sent the the target
 * @params <void*> resd : raw buffer of the response
 * @params <int> res_len : size of the response
 * @params <proto_result_t*> result : possibly set an error message in it to be parsed by execute_protocol
 *                                    (see in protocols.c for more information)
 * @return <int> : Return 0 if we got a response to our ping, -1 otherwise
 **/
int handle_icmp_packet(icmp_echo_t *req, void *req_pl, void *resd, int res_len, proto_result_t *result){
	if(*result != NO_ERROR){
		return -1;
	}

	icmp_echo_t *res = resd + IP_HEADER_SIZE;;
	void *res_pl = resd + IP_HEADER_SIZE + sizeof(icmp_echo_t);
	icmp_echo_t *orig_packet;
	
	int res_checksum = res->checksum;
	res->checksum = 0;
	
	int ret = -1;

	if(icmp_checksum(res, res_pl, res_len - sizeof(icmp_echo_t)) == res_checksum){
		switch(res->type){
			case ICMP_ECHO:
			if(req->identifier == res->identifier && req->sequence == res->sequence){
				if(memcmp(req_pl, res_pl, arguments.payload_size) == 0){
					ret = 0;
				} else {
					*result = ERROR_DATACORRUPT;
				}
			}
			break;
	
			case ICMP_DESTUNREACH:
			orig_packet = res_pl + IP_HEADER_SIZE;
			if(req->identifier == orig_packet->identifier && req->sequence == orig_packet->sequence){
				*result = ERROR_DESTUNREACH_NET + res->code;
			}
			break;
		}
	}

	return ret;
}

int icmp_recv(int sockd, void *payload, ping_time_t *start_time, float *latency, void *data){
	int size = (2 * IP_HEADER_SIZE + arguments.payload_size + sizeof(icmp_echo_t) * 2 + 64);
	void *buffer = malloc(size);
	int d;
	ping_time_t end_time;
	proto_result_t result = NO_ERROR;
	icmp_echo_t *req_header = data;

	do {
		do {
			d = recvfrom(sockd, buffer, size, MSG_DONTWAIT, NULL, NULL);
			if(d < 0 && errno != EAGAIN){
				result = ERROR_RECV;
			}

			end_time = time_ms();
			*latency = compute_latency(&end_time, start_time);
			if(*latency > arguments.timeoutMS){
				result = ERROR_TIMEOUT;
			}
		} while(d < 1 && result == NO_ERROR);	
	} while(handle_icmp_packet(req_header, payload, buffer, d, &result) < 0
			&& result == NO_ERROR);

	free(data);
	free(buffer);

	return result;
}

protocol_stack_t icmp_stack = {
	.sock    = &icmp_create_sock,
	.connect = &icmp_connect,
	.send    = &icmp_send,
	.recv    = &icmp_recv
};

