#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mutex.h"

static int mutex_lock(mutex_class *mutex)
{
    if (!mutex) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    if (pthread_mutex_lock(&(mutex->mux))) {
        PRINT_ERR("%s() mutex lock : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}
static int mutex_timelock(mutex_class *mutex, unsigned int ms)
{
    if (!mutex) {
        PRINT_ERR("%s() invalid args\n", __func__);
        return -1;
    }

    Timer tout;
    tm_set_ms(&tout, ms);

    int res = pthread_mutex_timedlock(&(mutex->mux), &tout);
    // Lock Timeout
    if (res == ETIMEDOUT) { return 1; }
    else { return 0; }

    PRINT_ERR("%s() mutex timedlock : %s\n", __func__, strerror(errno));
    return -1;
}
static int mutex_unlock(mutex_class *mutex)
{
    if (!mutex) {
        PRINT_ERR("%s() Invalid arg\n", __func__);
        return -1;
    }

    if (pthread_mutex_unlock(&(mutex->mux))) {
        PRINT_ERR("%s() mutex unlock : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

static int mutex_init(mutex_class *mutex)
{
    if (!mutex) {
        PRINT_ERR("%s() Invalid arg\n", __func__);
        return -1;
    }

    if (pthread_mutex_init(&(mutex->mux), NULL)) {
        PRINT_ERR("%s() mutex init : %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}
static int mutex_destroy(mutex_class *mutex)
{
    if (!mutex) {
        PRINT_ERR("%s() Invalid arg\n", __func__);
        return -1;
    }

    MUTEX_FREE(mutex);

    return 0;
}

mutex_class *mutex_new(void)
{
    mutex_class *ins = (mutex_class *)MUTEX_ALLOCATE("mutex obj", sizeof(mutex_class));
    if (!ins) { return NULL; }

    memset(ins, 0, sizeof(mutex_class));

    ins->init = mutex_init;
    ins->destroy = mutex_destroy;
    ins->lock = mutex_lock;
    ins->tmlock = mutex_timelock;
    ins->unlock = mutex_unlock;

    return ins;
}
