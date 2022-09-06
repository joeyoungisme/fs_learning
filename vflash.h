#ifndef __VIRTUAL_FLASH
#define __VIRTUAL_FLASH

#define VUNIT_TYPE                    unsigned int

typedef struct _vunit {
    unsigned char used;
    unsigned int erase;
    unsigned int write;
    VUNIT_TYPE data;
} vunit;

typedef struct _vpage {

    int (*init)(struct _vpage *, int);
    int (*destroy)(struct _vpage *);
    int (*size)(struct _vpage *);

    struct _vpage *(*dupc)(struct _vpage *);

    int (*erase)(struct _vpage *);
    int (*write)(struct _vpage *, int, const unsigned char *, int);
    int (*read)(struct _vpage *, int, unsigned char *, int);

    int (*dump)(struct _vpage *, int);
    int (*rdump)(struct _vpage *, int);
    int (*info)(struct _vpage *, int);

    unsigned char used;
    unsigned int vunit_amt;
    vunit *vunit;

} vpage;

typedef struct _vblock {

    int (*init)(struct _vblock *, int, int);
    int (*destroy)(struct _vblock *);

    struct _vblock *(*dupc)(struct _vblock *);

    int (*dump)(struct _vblock *, int);
    int (*rdump)(struct _vblock *, int);
    int (*wpage_dump)(struct _vblock *, int, vpage *, int);
    int (*wpage_rdump)(struct _vblock *, int, vpage *, int);
    int (*info)(struct _vblock *, int);

    int (*size)(struct _vblock *);
    int (*page_size)(struct _vblock *);

    vpage *(*get_page)(struct _vblock *, int);

    int (*erase)(struct _vblock *);
    int (*write)(struct _vblock *, int, const unsigned char *, int);
    int (*read)(struct _vblock *, int, unsigned char *, int);

    unsigned char used;
    unsigned int vunit_amt;
    unsigned int vpage_amt;
    vpage **vpage;
} vblock;

typedef struct _vflash {

    int (*init)(struct _vflash *, int, int, int);
    int (*destroy)(struct _vflash *);

    struct _vflash *(*dupc)(struct _vflash *);

    int (*size)(struct _vflash *);
    int (*block_size)(struct _vflash *);
    int (*page_size)(struct _vflash *);
    int (*unit_size)(struct _vflash *);

    vblock *(*get_block)(struct _vflash *, int);

    int (*fdump)(struct _vflash *);
    int (*fload)(struct _vflash *, char *);

    int (*fimport)(struct _vflash *, char *);
    int (*fexport)(struct _vflash *, char *);

    int (*fblock_update)(struct _vflash *, int);
    int (*fpage_update)(struct _vflash *, int, int);

    unsigned char used;
    unsigned int vunit_amt;
    unsigned int vpage_amt;
    unsigned int vblock_amt;

    char file_name[64];

    vblock **vblock;

} vflash;

vpage *vpage_new();
vblock *vblock_new();
vflash *vflash_new();

#endif
