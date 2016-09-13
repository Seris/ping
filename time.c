#include <stdlib.h>
#include <sys/time.h>

#include <time_ms.h>

/**
 * time_ms Return the time in µsec. The name does not make any sense in regards
 * of what it's doing absolutely doing, but it makes sense when you use it with
 * compute latency.
 * @return <ping_time_t>
 */
ping_time_t time_ms(void){
	struct timeval st;
	gettimeofday(&st, NULL);
	return (ping_time_t) st;
}

/**
 * compute_latency Compute time between end and start in ms
 * @params <ping_time_t*> end, start : Get this from time_ms()
 * @return <float> time between end and start in ms
 **/
float compute_latency(ping_time_t *end, ping_time_t *start){
	float t = 0;

	t += (end->tv_sec - start->tv_sec) * 1000;
	t += (float)(end->tv_usec - start->tv_usec) / 1000;
	return t;
}

