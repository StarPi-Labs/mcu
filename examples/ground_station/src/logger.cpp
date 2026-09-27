#include <time.h>
#include "logger.h"

uint64_t now_us(void)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	return (uint64_t)tv_now.tv_sec * 1000000ULL + tv_now.tv_usec;
}


uint64_t now_ms(void)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	return (uint64_t)tv_now.tv_sec * 1000ULL + tv_now.tv_usec / 1000;
}
