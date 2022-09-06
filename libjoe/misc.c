#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <stdarg.h>
#include <uuid/uuid.h>

#include "misc.h"

int run_cmd(char *key, cmd_callback func, char *fmt, ...)
{
    if (!key || !func || !fmt) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    char cmd[256] = {0};

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(cmd, 256, fmt, arg);
    va_end(arg);

    PRINT_DEBUG("%s() : %s\n", __func__, cmd);

    FILE *fd = popen(cmd, "r");
    if (!fd) {
        PRINT_ERR("%s() popen : %s\n", __func__, strerror(errno));
        return -1;
    }

    char line[512] = {0};
    while(fgets(line, sizeof(line), fd)) {

        if (!strstr(line, key)) { continue; }

        func(line);
    }

    int res = pclose(fd);

    if (res) {
        PRINT_ERR("%s() pclose : %d (%i) [%s]\n", __func__, res, WEXITSTATUS(res), strerror(errno));
        return -1;
    }

    return 0;
}

int exe_cmd(char *fmt, ...)
{
    char cmd[256] = {0};

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(cmd, 256, fmt, arg);
    va_end(arg);

    PRINT("%s() : %s\n", __func__, cmd);

    int res = system(cmd);
    if (res) {
        PRINT_ERR("%s() system cmd : %s\n", __func__, cmd);
        PRINT_ERR("%s() system res : %d , errno = %d.\n", __func__, res, errno);
    }
    return res;
}

int epoch2utc(time_t epoch, char *utc, int len)
{
    if(!utc || !len) return 0;

    memset(utc, 0, len);
    time_t ep = epoch;
    struct tm *tm = localtime(&ep);
    int res =  snprintf(utc, len, "%4d-%02d-%02dT%02d:%02d:%02dZ",
                        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec);
    return res;
}
