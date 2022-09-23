#ifndef MISC_MISC_H
#define MISC_MISC_H

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#endif

#define BIT_MASK(x)             ((0x01) << x)

#define VA_ARGS(...)     , ##__VA_ARGS__

#ifdef DEBUG_FLAG

#include <time.h>
#include <syslog.h>

#define PRINT(fmt, ...) \
    do { printf("["__FILE__"] [LINE %d] "fmt, __LINE__ VA_ARGS(__VA_ARGS__)); } while(0)

#define PRINT_RAW(...) \
    do { printf(__VA_ARGS__); } while(0)

#define PRINT_LOG(fmt, ...) \
    do { syslog(LOG_INFO, "["__FILE__"] [LINE %d] "fmt, __LINE__ VA_ARGS(__VA_ARGS__)); } while(0)

#define PRINT_ERR(fmt, ...) \
    do { printf("[ERROR] ["__FILE__"] [LINE %d] "fmt, __LINE__ VA_ARGS(__VA_ARGS__)); } while(0)

#define PRINT_DEBUG(fmt, ...) \
    do { printf("[DEBUG] ["__FILE__"] [LINE %d] "fmt, __LINE__ VA_ARGS(__VA_ARGS__)); } while(0)

#else

#define PRINT(...)
#define PRINT_RAW(...) \
    do { printf(__VA_ARGS__); } while(0)
#define PRINT_ERR(fmt, ...) \
    do { printf("[ERROR] ["__FILE__"] [LINE %d] "fmt, __LINE__ VA_ARGS(__VA_ARGS__)); } while(0)
#define PRINT_DEBUG(...)

#endif

/*
#define NewFunc(prefix, last)           prefix##last()
#define New(type, var, err_act, ...) \
    type *var = NewFunc(type, New); \
    if (!var) { \
        PRINT_ERR("%s() "#type" New failed\n", __func__); \
        err_act; \
    } else if (var->init(var, __VA_ARGS__)) { \
        PRINT_ERR("%s() "#type" "#var" init error\n", __func__); \
        err_act; \
    }
*/

#define DiffTimeb(start, end) \
    ((((uint32_t)end.time - (uint32_t)start.time) * 1000) + (end.millitm - start.millitm))

typedef void (*cmd_callback)(char *line);

int run_cmd(char *key, cmd_callback func, char *fmt, ...);
int exe_cmd(char *, ...);
int epoch2utc(time_t, char *, int);

#endif


