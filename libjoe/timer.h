#ifndef MISC_TIMER_H
#define MISC_TIMER_H

#include <time.h>

typedef struct timespec Timer;

// prevent time sync error ... casued infinity Timer ...
#define TIMER_MAX_INTERVAL              (86400000)

int tm_set_ms(Timer *tmr, unsigned int ms);
int tm_is_timeout(Timer tmr);
int tm_interval(Timer tm1, Timer tm2);
int tm_remaining(Timer tmr);
int tm_stopwatch(Timer tmr);

int tm_is_exprid(Timer tmr);
int tm_from_utc(Timer *tmr);
int tm_to_utc(Timer *tmr);
int tm_to_sys(Timer *tmr);
int tm_from_sys(Timer *tmr);

int tm_to_str(Timer *tmr, char *str, int len);
int tm_from_str(Timer *tmr, char *str);
int tm_to_fattime(Timer *tmr, unsigned long *fattime);

#endif
