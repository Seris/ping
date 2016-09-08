#include <stdlib.h>
#include <sys/time.h>

#include <time_ms.h>

ping_time_t time_ms(void){
	struct timeval st;
	gettimeofday(&st, NULL);
	return (ping_time_t) st;
}

float compute_latency(ping_time_t *end, ping_time_t *start){
	float t = 0;

	t += (end->tv_sec - start->tv_sec) * 1000;
	t += (float)(end->tv_usec - start->tv_usec) / 1000;
	return t;
}

