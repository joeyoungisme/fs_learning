#ifndef __MYFAT_H
#define __MYFAT_H

#include "ff.h"
#include "ff_gen_drv.h"
#include "vflash.h"

#include <stddef.h>
#define container_of(ptr, type, member) \
    ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); \
     (type *)( (char *)__mptr - offsetof(type,member) ); })

typedef struct _myfat_dev {

    int (*init)(struct _myfat_dev *, int, int, int);
    int (*destroy)(struct _myfat_dev *);

    // for multi dev situation
    int (*change)(struct _myfat_dev *);

    int (*save)(struct _myfat_dev *, char *);

    int id;
    int clus_size;
    char path[16];

    DSTATUS state;
    FATFS core;
    Diskio_drvTypeDef *config;
    vflash *flash;

    // for our method and attribute
    int read_cnt;
    int prog_cnt;
    int erase_cnt;
    int sync_cnt;

    struct _myfat_dev *next;

} myfat_dev;

myfat_dev *myfat_dev_new(void);

typedef struct _myfat {

    int (*init)(struct _myfat *);
    int (*destroy)(struct _myfat *);

    int (*list)(struct _myfat *);
    int (*new_dev)(struct _myfat *, int, int, int);
    int (*rm_dev)(struct _myfat *, int);

    int (*format)(struct _myfat *, int);
    int (*mount)(struct _myfat *, int);
    int (*unmount)(struct _myfat *, int);
    int (*mkdir)(struct _myfat *, int, DIR *, char *);
    int (*opendir)(struct _myfat *, int, DIR *, char *);
    int (*readdir)(struct _myfat *, int, DIR *, FILINFO *);
    int (*closedir)(struct _myfat *, int, DIR *);
    int (*open)(struct _myfat *, int, FIL *, char *, int);
    int (*close)(struct _myfat *, int, FIL *);
    int (*write)(struct _myfat *, int, FIL *, unsigned char *, int);
    int (*read)(struct _myfat *, int, FIL *, unsigned char *, int);
    int (*sync)(struct _myfat *, int, FIL *);
    int (*remove)(struct _myfat *, int, char *);
    int (*save)(struct _myfat *, int);

    int (*get_free_clust)(struct _myfat *, int, unsigned int *);

    vflash *(*get_flash)(struct _myfat *, int);
    int (*get_read_cnt)(struct _myfat *, int);
    int (*get_prog_cnt)(struct _myfat *, int);
    int (*get_erase_cnt)(struct _myfat *, int);
    int (*get_sync_cnt)(struct _myfat *, int);
    void (*clear_cnt)(struct _myfat *, int);

    myfat_dev *head;

} myfat;

myfat *myfat_new(void);

// for fat_core get RTC time
DWORD get_fattime(void);
myfat_dev *find_fat_by_id(BYTE id);

#endif
