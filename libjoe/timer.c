#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>

// for strptime
#define __USE_XOPEN
#include <time.h>

#include "misc.h"
#include "timer.h"

static int tm_add_ns(Timer *timer, int ns)
{
    if (!timer) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    timer->tv_nsec += ns;

    if (timer->tv_nsec > 1000000000) {
        timer->tv_sec++;
        timer->tv_nsec %= 1000000000;
    }

    return 0;
}

static int tm_intv_ms(Timer start, Timer stop)
{
    int intv_ms = 0;
    if ((stop.tv_nsec - start.tv_nsec) < 0) {
        intv_ms = (stop.tv_sec - start.tv_sec - 1) * 1000;
        intv_ms += (stop.tv_nsec - start.tv_nsec + 1000000000) / 1000000;
    } else {
        intv_ms = (stop.tv_sec - start.tv_sec) * 1000;
        intv_ms += (stop.tv_nsec - start.tv_nsec) / 1000000;
    }
    return intv_ms;
}

int tm_is_timeout(Timer tmr)
{
    Timer curr = {0};
    clock_gettime(CLOCK_MONOTONIC, &curr);

    int intv = tm_intv_ms(curr, tmr);
    if (intv <= 0) {
        return 1;
    } else if (intv >= TIMER_MAX_INTERVAL) {
        return 1;
    }

    return 0;
}

int tm_interval(Timer tm1, Timer tm2)
{
    return tm_intv_ms(tm1, tm2);
}

// interval : current time --> input time 
int tm_remaining(Timer tmr)
{
    Timer curr = {0};
    clock_gettime(CLOCK_MONOTONIC, &curr);

    return tm_intv_ms(curr, tmr);
}

int tm_stopwatch(Timer tmr)
{
    Timer curr = {0};
    clock_gettime(CLOCK_MONOTONIC, &curr);

    return tm_intv_ms(tmr, curr);
}

int tm_set_ms(Timer *tmr, unsigned int ms)
{
    if (!tmr) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    Timer curr = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &curr)) {
        PRINT_ERR("%s() clock_gettime : %s\n", __func__, strerror(errno));
        return -1;
    }

    memcpy(tmr, &curr, sizeof(Timer));

    tmr->tv_sec += (ms / 1000);
    ms %= 1000;
    tmr->tv_nsec += (ms * 1000000);
    tmr->tv_sec += (tmr->tv_nsec / 1000000000);
    tmr->tv_nsec %= 1000000000;

    return 0;
}

int tm_to_str(Timer *tmr, char *str, int len)
{
    if (!tmr || !str || !len) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (len < 32) {
        PRINT_ERR("%s() WARNING len too less.\n", __func__);
    }

    struct tm tmp = {0};
    gmtime_r(&(tmr->tv_sec), &tmp);
    strftime(str, 21, "%Y-%m-%dT%H:%M:%S.", &tmp);
    snprintf(str + 21 - 1, len - 21, "%09luZ", tmr->tv_nsec);

    return 0;
}

int tm_from_str(Timer *tmr, char *str)
{
    if (!tmr || !str) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    struct tm tmp = {0};
    if (!strptime(str, "%Y-%m-%dT%H:%M:%S", &tmp)) {
        PRINT_ERR("%s() strptime : %s\n", __func__, strerror(errno));
        PRINT_ERR("%s() input string : %s\n", __func__, str);
        return -1;
    }

    tmr->tv_sec = timegm(&tmp);

    // nanosecond
    char date[32] = {0};
    char nano[32] = {0};
    if (sscanf(str, "%[^.].%[0-9]", date, nano) == 2) {
        int nanosecond = atoi(nano);
        tm_add_ns(tmr, nanosecond);
    } else {
        tmr->tv_nsec = 0;
    }

    return 0;
}

int tm_from_utc(Timer *tmr)
{
    if (!tmr) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (clock_gettime(CLOCK_REALTIME, tmr)) {
        PRINT_ERR("%s() clock_gettime : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

int tm_to_utc(Timer *tmr)
{
    if (!tmr) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (clock_settime(CLOCK_REALTIME, tmr)) {
        PRINT_ERR("%s() clock_settime : %s\n", __func__, strerror(errno));
        return -1;
    }
    
    exe_cmd("hwclock -w -f /dev/rtc1");

    return 0;
}

int tm_from_sys(Timer *tmr)
{
    if (!tmr) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    } else if (tm_from_utc(tmr)) {
        PRINT_ERR("%s() tm from utc failed.\n", __func__);
        return -1;
    }

    int offset = (int)(-timezone / 60 + (daylight ? 60:0));
    tmr->tv_sec += (offset * 60);

    return 0;
}

int tm_to_sys(Timer *tmr)
{
    if (!tmr) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    int offset = (int)(-timezone / 60 + (daylight ? 60:0));
    tmr->tv_sec -= (offset * 60);

    if (tm_to_utc(tmr)) {
        PRINT_ERR("%s() tm to utc failed.\n", __func__);
        return -1;
    }

    return 0;
}

int tm_is_exprid(Timer tmr)
{
    Timer curr = {0};
    tm_from_sys(&curr);

    int intv = tm_intv_ms(curr, tmr);
    if (intv <= 0) {
        return 1;
    } else if (intv >= TIMER_MAX_INTERVAL) {
        return 1;
    }

    return 0;
}
