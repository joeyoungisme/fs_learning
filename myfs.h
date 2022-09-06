#ifndef __myfs_H
#define __myfs_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "timer.h"
#include "vflash.h"

#include "mylfs.h"
#include "myfat.h"


#define FAT_ROOT            "fat:/"
#define LFS_ROOT            "lfs:/"

typedef enum _myfs_type {
    MYFS_LITTLEFS = 0,
    MYFS_FATFS,
    MYFS_TYPE_AMOUNT
} MYFS_TYPE;

typedef union _myfs_file {
    FIL fat;
    lfs_file_t lfs;
} myfs_file;

typedef union _myfs_dir {
    DIR fat;
    lfs_dir_t lfs;
} myfs_dir;

typedef union _myfs_file_info {
    FILINFO fat;
    struct lfs_info lfs;
} myfs_file_info;

typedef struct _myfs {

    int (*init)(struct _myfs *, MYFS_TYPE, int, int, int);
    int (*destroy)(struct _myfs *);

    int (*format)(struct _myfs *);
    int (*mount)(struct _myfs *);
    int (*unmount)(struct _myfs *);
    int (*mkdir)(struct _myfs *, char *);
    int (*closedir)(struct _myfs *, myfs_dir *);
    int (*readdir)(struct _myfs *, myfs_dir *, myfs_file_info *);
    int (*opendir)(struct _myfs *, myfs_dir *, char *);
    int (*open)(struct _myfs *, myfs_file *, char *, int);
    int (*close)(struct _myfs *, myfs_file *);
    int (*write)(struct _myfs *, myfs_file *, unsigned char *, int);
    int (*read)(struct _myfs *, myfs_file *, unsigned char *, int);
    int (*remove)(struct _myfs *, char *);
    void (*list)(struct _myfs *, char *);
    void (*save)(struct _myfs *);

    // for fat only method
    int (*get_free_clust)(struct _myfs *, unsigned int *);

    // for lfs only method

    vflash *(*get_flash)(struct _myfs *);
    int (*get_read_cnt)(struct _mylfs *);
    int (*get_prog_cnt)(struct _mylfs *);
    int (*get_erase_cnt)(struct _mylfs *);
    int (*get_sync_cnt)(struct _mylfs *);
    void (*clear_cnt)(struct _mylfs *);

    MYFS_TYPE type;

    myfat *fat;
    mylfs *lfs;

} myfs;

myfs *myfs_new(void);

/* USER CODE END Prototypes */
#ifdef __cplusplus
}
#endif
#endif /*__myfs_H */
