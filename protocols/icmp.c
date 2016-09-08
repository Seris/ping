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

int icmp_send(int sockd, void *payload, ping_time_t *start_time, void **data){
	int result = NO_ERROR;
	void *buffer;
	icmp_echo_t *header = malloc(sizeof(icmp_echo_t));
	int size = icmp_create_packet(header, payload, &buffer);
	if(send(sockd, buffer, size, 0) < 0){
		result = ERROR_SEND;
		free(header);
	} else {
		*data = header;
		*start_time = time_ms();
	}
	free(buffer);
	return result;
}

int icmp_recv(int sockd, void *payload, ping_time_t *start_time, float *latency, void *data){
	int size = IP_HEADER_SIZE + arguments.payload_size + sizeof(icmp_echo_t);
	void *buffer = malloc(size);
	void *cursor = buffer;
	int recv_len = 0, d;
	ping_time_t end_time;
	proto_result_t result = NO_ERROR;
	icmp_echo_t *req_header = data;
	icmp_echo_t *res_header;
	void *res_payload;

	do {
		do {
			d = recv(sockd, cursor, size - recv_len, MSG_DONTWAIT);
			if(d < 0){
				if(errno != EAGAIN){
					result = ERROR_RECV;
				}
			} else {
				recv_len += d;
			}
		
			end_time = time_ms();
			*latency = compute_latency(&end_time, start_time);
			if(*latency > arguments.timeoutMS){
				result = ERROR_TIMEOUT;
			}
		} while(recv_len < size && result == NO_ERROR);
		res_header = buffer + IP_HEADER_SIZE;
		res_payload = buffer + IP_HEADER_SIZE + sizeof(icmp_echo_t);
	} while(req_header->identifier != res_header->identifier &&
			res_header->sequence != req_header->sequence);

	if(result == NO_ERROR && 
	  (memcmp(res_payload, payload, arguments.payload_size) != 0 ||
	   icmp_checksum(res_header, res_payload, arguments.payload_size) == req_header->checksum)){
		result = ERROR_DATACORRUPT;
	}

	free(buffer);
	free(data);

	return result;
}

protocol_stack_t icmp_stack = {
	.sock    = &icmp_create_sock,
	.connect = &udptcp_connect,
	.send    = &icmp_send,
	.recv    = &icmp_recv
};

