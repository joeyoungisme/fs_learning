#ifndef __MY_FILESYSTEM
#define __MY_FILESYSTEM

#include <stddef.h>
#include "lfs.h"
#include "vflash.h"

#define container_of(ptr, type, member) \
    ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); \
       (type *)( (char *)__mptr - offsetof(type,member) ); })

typedef struct _mylfs_dev {

    int (*init)(struct _mylfs_dev *, int, int, int);
    int (*destroy)(struct _mylfs_dev *);

    int (*save)(struct _mylfs_dev *, char *);

    int id;
    lfs_t core;
    struct lfs_config config;
    vflash *flash;

    // for our method and attribute
    int read_cnt;
    int prog_cnt;
    int erase_cnt;
    int sync_cnt;

    struct _mylfs_dev *next;

} mylfs_dev;

mylfs_dev *mylfs_dev_new(void);

typedef struct _mylfs {

    int (*init)(struct _mylfs *);
    int (*destroy)(struct _mylfs *);

    int (*list)(struct _mylfs *);
    int (*new_dev)(struct _mylfs *, int, int, int);
    int (*rm_dev)(struct _mylfs *, int);

    int (*format)(struct _mylfs *, int);
    int (*mount)(struct _mylfs *, int);
    int (*unmount)(struct _mylfs *, int);
    int (*mkdir)(struct _mylfs *, int, lfs_dir_t *, char *);
    int (*opendir)(struct _mylfs *, int, lfs_dir_t *, char *);
    int (*readdir)(struct _mylfs *, int, lfs_dir_t *, struct lfs_info *);
    int (*closedir)(struct _mylfs *, int, lfs_dir_t *);
    int (*open)(struct _mylfs *, int, lfs_file_t *, char *, int);
    int (*close)(struct _mylfs *, int, lfs_file_t *);
    int (*write)(struct _mylfs *, int, lfs_file_t *, unsigned char *, int);
    int (*read)(struct _mylfs *, int, lfs_file_t *, unsigned char *, int);
    int (*sync)(struct _mylfs *, int, lfs_file_t *);
    int (*remove)(struct _mylfs *, int, char *);
    int (*save)(struct _mylfs *, int);

    void (*clear_cnt)(struct _mylfs *, int);
    int (*get_sync_cnt)(struct _mylfs *, int);
    int (*get_erase_cnt)(struct _mylfs *, int);
    int (*get_prog_cnt)(struct _mylfs *, int);
    int (*get_read_cnt)(struct _mylfs *, int);
    vflash *(*get_flash)(struct _mylfs *, int);

    mylfs_dev *head;

} mylfs;

mylfs *mylfs_new(void);

#endif

