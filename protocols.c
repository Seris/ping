#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>
#include <float.h>

#include <arguments.h>
#include <protocols.h>
#include <time_ms.h>

extern args_t arguments;
extern protocol_stack_t udp_stack, tcp_stack, icmp_stack;

void gen_random_payload(uint8_t *payload){
	srand(time(NULL));
	bzero(payload, arguments.payload_size);
	for(int i = 0; i < arguments.payload_size; i++){
		payload[i] = (uint8_t) rand();
	}
}

int execute_protocol(protocol_stack_t *stack){
	int sockd, count = 1, error;
	uint8_t *payload = (uint8_t*) malloc(arguments.payload_size);
	float latency, average_latency = 0, min_latency = FLT_MAX, max_latency = -1;
	ping_time_t start_time;
	uint32_t count_response = 0, count_error = 0;
	void *data;

	printf("PING %s - %d bytes of data:\n", arguments.ip_address, arguments.payload_size);

	do {
		error = NO_ERROR;
		sockd = stack->sock();

		if(sockd == ERROR_SOCK){
			error = ERROR_SOCK;
		}

		if(error == NO_ERROR){
			error = stack->connect(sockd);
		}

		if(error == NO_ERROR){
			gen_random_payload(payload);
			error = stack->send(sockd, payload, &start_time, &data);
		}

		if(error == NO_ERROR){
			error = stack->recv(sockd, payload, &start_time, &latency, data);
		}

		if(error == NO_ERROR){
			average_latency += latency;
			count_response++;

			if(latency > max_latency){
				max_latency = latency;
			}

			if(latency < min_latency){
				min_latency = latency;
			}
			
			printf("#%d response from %s (latency=%.2fms, size=%dB)\n",
				count, arguments.ip_address, latency, arguments.payload_size);

			if(sockd != ERROR_SOCK){
				close(sockd);
			}
		} else {
			print_error(error, count);
			count_error++;
		}

		sleep(arguments.interval);

	} while(++count <= arguments.count);

	printf("---- statistics from %s ----\n", arguments.ip_address);
	if(count_response > 0){
		average_latency /= count_response;
		printf("latency: min=%.2fms, max=%.2fms, average=%.2fms\n", min_latency, max_latency, average_latency);
		printf("packets: transmitted=%d, failed=%d, total=%d\n", count_response, count_error, arguments.count);
	} else {
		printf("no packet transmitted\n");
	}
}

protocol_stack_t* get_protocol_stack(void){
	protocol_stack_t* stack;
	switch(arguments.protocol){
		case PROTOCOL_UDP:
		stack = &udp_stack;
		break;

		case PROTOCOL_TCP:
		stack = &tcp_stack;
		break;

		case PROTOCOL_ICMP:
		stack = &icmp_stack;
		break;

		default:
		stack = NULL;
		break;
	}

	return stack;
}

void print_error(int error, int count){
	printf("#%d error::", count);
	fflush(stdout);
	switch(error){
		case ERROR_RECV:
		perror("recv Could not receive data");
		break;

		case ERROR_SEND:
		perror("send No data could be sent");
		break;
		
		case ERROR_CONNECT:
		perror("connect Could not connect to host");
		break;

		case ERROR_SOCK:
		perror("socket Could not create socket");
		break;

		case ERROR_DATACORRUPT:
		printf("corruption Received data is corrupted\n");
		break;

		case ERROR_TIMEOUT:
		printf("timeout Cound not contact host (latency>%dms)\n", arguments.timeoutMS);
		break;

		case ERROR_DESTUNREACH_NET:
		printf("unreachable Net Unreachable\n");
		break;

		case ERROR_DESTUNREACH_HOST:
		printf("unreachable Host Unreachable\n");
		break;

		case ERROR_DESTUNREACH_PROTO:
		printf("unreachable Protocol Unreachable\n");
		break;

		case ERROR_DESTUNREACH_PORT:
		printf("unreachable Port Unreachable\n");
		break;

		case ERROR_DESTUNREACH_FRAG:
		printf("unreachable Fragmentation needed but DF flags is set\n");
		break;

		case ERROR_DESTUNREACH_ROUTE:
		printf("unreachable Source Route Failed\n");
		break;

		default:
		printf("error An error as occured (code: %d)\n", error);
	}
}
