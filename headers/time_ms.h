#ifndef TIME_MS_H
#define TIME_MS_H

typedef struct timeval ping_time_t;

ping_time_t time_ms(void);
float compute_latency(ping_time_t *end, ping_time_t *start);

#endif
