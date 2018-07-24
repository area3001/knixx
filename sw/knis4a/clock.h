#ifndef CLOCK_H_
#define CLOCK_H_

#define CLOCK_SYSTICK_FREQ 1000UL
#define CLOCK_STR_LEN 12

struct clock {
	unsigned long secs;
	unsigned short msecs;
};

void clock_setup(void);
void clock_timestamp(char *str);

#endif
