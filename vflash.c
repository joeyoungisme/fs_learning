#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "misc.h"
#include "vflash.h"

// joe printf
#define va_args(...)        , ##__VA_ARGS__
#define jprintf(fmt, ...) \
    do { \
        printf("%s [LINE %d] : " fmt, __func__, __LINE__ va_args(__VA_ARGS__)); \
    } while(0)

static void vunit_reverse(VUNIT_TYPE *vu)
{
    if (!vu) { return; }

    int vu_size = sizeof(VUNIT_TYPE);
    unsigned char *pvu = (unsigned char *)vu;
    for (int idx = 0; idx < (vu_size / 2); ++idx)
    {
        unsigned char temp = pvu[idx];
        pvu[idx] = pvu[vu_size - 1 - idx];
        pvu[vu_size - 1 - idx] = temp;
    }

    return;
}

// ---- vpage ----
vpage *vpage_dupc(vpage *page)
{
    if (!page) {
        jprintf("invaild args.\n");
        return NULL;
    }

    vpage *page_new = vpage_new();
    if (!page_new) {
        jprintf("page new failed\n");
        return NULL;
    } else if (page_new->init(page_new, page->vunit_amt)) {
        jprintf("page init failed\n");
        page_new->destroy(page_new);
        return NULL;
    }

    memcpy(page->vunit, page_new->vunit, sizeof(VUNIT_TYPE) * page->vunit_amt);

    return page_new;
}
int vpage_dump(vpage *page, int unit_per_line)
{
    if (!page || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    int unit_cnt = 0;

    // -------------------------------------------------- ... 
    // | 00000000 - 00000000 | xxxxxxxx xxxxxxxx xxxxxxxx ... 
    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-");
    for (int idx = 0; ; ++idx)
    {
        if (!(idx % unit_per_line)) {
            if (idx) { printf("|"); }
            printf("\n");
            printf("| %08d - %08d | ", idx, (idx + unit_per_line));
        }

        printf("%08X ", page->vunit[unit_cnt].data);

        if (++unit_cnt == page->vunit_amt) {
            int left = (unit_per_line - (idx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("         ");
                }
            }
            printf("|\n");
            break;
        }
    }

    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vpage_dump_read(vpage *page, int unit_per_line)
{
    if (!page || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    int unit_cnt = 0;

    // -------------------------------------------------- ... 
    // | 00000000 - 00000000 | xxxxxxxx xxxxxxxx xxxxxxxx ... 
    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-");
    for (int idx = 0; ; ++idx)
    {
        if (!(idx % unit_per_line)) {
            if (idx) { printf("|"); }
            printf("\n");
            printf("| %08d - %08d | ", idx, (idx + unit_per_line));
        }

        VUNIT_TYPE data = page->vunit[unit_cnt].data;
        unsigned char *shw = (unsigned char *)&data;
        shw += (sizeof(VUNIT_TYPE) - 1);
        for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
            if ((*shw > ' ') && (*shw < '~')) {
                printf(".%c", *shw);
            } else {
                printf("%02X", *shw);
            }
            shw--;
        }
        printf(" ");

        // printf("%08X ", page->vunit[unit_cnt].data;

        if (++unit_cnt == page->vunit_amt) {
            int left = (unit_per_line - (idx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("         ");
                }
            }
            printf("|\n");
            break;
        }
    }

    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vpage_info(vpage *page, int unit_per_line)
{
    if (!page || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    int unit_cnt = 0;

    // ----------------------------------------------------------- ... 
    // | 00000000 - 00000000 | [■ (xx,oo)] [■ (xx,oo)] [■ (xx,oo)] ... 
    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("------------");
    printf("-");
    for (int idx = 0; ; ++idx)
    {
        if (!(idx % unit_per_line)) {
            if (idx) { printf("|"); }
            printf("\n");
            printf("| %08d - %08d | ", idx, (idx + unit_per_line));
        }

        if (page->vunit[unit_cnt].used) {
            printf("[■ (%02d,%02d)] ",
                    page->vunit[unit_cnt].erase,
                    page->vunit[unit_cnt].write);
        } else {
            printf("[□ (%02d,%02d)] ",
                    page->vunit[unit_cnt].erase,
                    page->vunit[unit_cnt].write);
        }

        if (++unit_cnt == page->vunit_amt) {
            int left = (unit_per_line - (idx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("            ");
                }
            }
            printf("|\n");
            break;
        }
    }

    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("------------");
    printf("-\n");

    return 0;
}

int vpage_erase(vpage *page)
{
    if (!page) {
        jprintf("invalid args\n");
        return -1;
    }

    for (int idx = 0; idx < page->vunit_amt; ++idx) {
        page->vunit[idx].used = 0;
        page->vunit[idx].erase++;
        page->vunit[idx].data = (VUNIT_TYPE)-1;
    }

    return 0;
}
int vpage_read(vpage *page, int off, unsigned char *buffer, int size)
{
    if (!page || !buffer || !size) {
        jprintf("invalid args\n");
        return -1;
    } else if ((off + size) > page->size(page)) {
        jprintf("overflow \n");
        return -1;
    }

    int align = sizeof(VUNIT_TYPE);
    if (off % align) {
        printf("off no align.\n");
        int no_align = align - (off % align);
        int unit_index = off / align;
        if (page->vunit[unit_index].used) {
            printf("over write.\n");
            return -1;
        }
        page->vunit[unit_index].write++;
        for (int idx = 0; idx < no_align; ++idx) {
            buffer[idx] = page->vunit[unit_index].data >> ((align - no_align + idx) * 8);
        }
        off += no_align;
        size -= no_align;
    }

    if (size % align) {
        printf("size no align.\n");
        int no_align = size % align;
        int unit_index = (off + size) / align;
        if (page->vunit[unit_index].used) {
            printf("over write.\n");
            return -1;
        }
        page->vunit[unit_index].write++;
        for (int idx = 0; idx < no_align; ++idx) {
            buffer[size - no_align + idx] = page->vunit[unit_index].data >> ((align - no_align + idx) * 8);
        }
        size -= no_align;
    }

    int cnt = size / align;
    VUNIT_TYPE *vbuff = (VUNIT_TYPE *)buffer;
    for (int idx = 0; idx < cnt; ++idx) {
        *vbuff = page->vunit[(off / align) + idx].data;
        vunit_reverse(vbuff);
        vbuff++;
    }

    return 0;
}
int vpage_write(vpage *page, int off, const unsigned char *buffer, int size)
{
    if (!page || !buffer || !size) {
        jprintf("invalid args\n");
        return -1;
    } else if ((off + size) > page->size(page)) {
        jprintf("overflow \n");
        return -1;
    }

    int align = sizeof(VUNIT_TYPE);
    if (off % align) {
        printf("off no align.\n");
        int no_align = align - (off % align);
        int unit_index = off / align;
        if (page->vunit[unit_index].used) {
            printf("over write.\n");
            return -1;
        }
        page->vunit[unit_index].write++;
        page->vunit[unit_index].used = 1;
        for (int idx = 0; idx < no_align; ++idx) {
            page->vunit[unit_index].data &= ~(0xFF << ((align - no_align + idx) * 8));
            page->vunit[unit_index].data |= (buffer[idx] << ((align - no_align + idx) * 8));
        }
        off += no_align;
        size -= no_align;
    }

    if (size % align) {
        printf("size no align.\n");
        int no_align = size % align;
        int unit_index = (off + size) / align;
        if (page->vunit[unit_index].used) {
            printf("over write.\n");
            return -1;
        }
        page->vunit[unit_index].write++;
        page->vunit[unit_index].used = 1;
        for (int idx = 0; idx < no_align; ++idx) {
            page->vunit[unit_index].data &= ~(0xFF << ((align - no_align + idx) * 8));
            page->vunit[unit_index].data |= (buffer[size - no_align + idx] << (idx * 8));
        }
        size -= no_align;
    }

    int cnt = size / align;
    VUNIT_TYPE *vbuff = (VUNIT_TYPE *)buffer;
    for (int idx = 0; idx < cnt; ++idx) {
        if (*vbuff != (VUNIT_TYPE)-1) {
            page->vunit[(off / align) + idx].write++;
            page->vunit[(off / align) + idx].used = 1;
            page->vunit[(off / align) + idx].data = *vbuff;
            vunit_reverse(&(page->vunit[(off / align) + idx].data));
        }
        vbuff++;
    }

    page->used = 1;

    return 0;
}
int vpage_size(vpage *page)
{
    if (!page) { return 0; }

    return page->vunit_amt * sizeof(VUNIT_TYPE);
}
int vpage_init(vpage *page, int vunit_amt)
{
    if (!page || !vunit_amt) {
        jprintf("invaild args.\n");
        return -1;
    }

    page->used = 0;
    page->vunit_amt = vunit_amt;
    page->vunit = (vunit *)malloc(sizeof(vunit) * vunit_amt);
    if (!page->vunit) {
        jprintf("page new unit failed\n");
        return -1;
    }

    for (int idx = 0; idx < page->vunit_amt; ++idx) {
        page->vunit[idx].used = 0;
        page->vunit[idx].erase = 0;
        page->vunit[idx].write = 0;
        page->vunit[idx].data = (VUNIT_TYPE)-1;
    }

    return 0;
}

int vpage_destroy(vpage *page)
{
    if (!page) {
        jprintf("invaild args.\n");
        return -1;
    } else if (!page->vunit) {
        return 0;
    } else if (!page->vunit_amt) {
        return 0;
    }

    free(page->vunit);
    page->vunit = NULL;
    free(page);

    return 0;
}
vpage *vpage_new(void)
{
    vpage *page = (vpage *)malloc(sizeof(vpage));
    if (!page) {
        jprintf("allocate failed\n");
        return NULL;
    }
    memset(page, 0, sizeof(vpage));

    // assign page method
    page->init    = vpage_init;
    page->destroy = vpage_destroy;
    page->size    = vpage_size;
    page->erase   = vpage_erase;
    page->write   = vpage_write;
    page->read    = vpage_read;
    page->dump    = vpage_dump;
    page->dupc    = vpage_dupc;
    page->rdump   = vpage_dump_read;
    page->info    = vpage_info;

    return page;
}

// ---- vblock ----

vpage *vblock_get_page(vblock *block, int page_num)
{
    if (!block || (page_num >= block->vpage_amt)) {
        jprintf("invalid args\n");
        return NULL;
    } else if (!block->vpage) {
        jprintf("no init\n");
        return NULL;
    }

    return block->vpage[page_num];
}
int vblock_erase(vblock *block)
{
    if (!block) {
        jprintf("invalid args\n");
        return -1;
    }
    for (int idx = 0; idx < block->vpage_amt; ++idx)
    {
        vpage *page = block->get_page(block, idx);
        if (page) {
            page->erase(page);
        }
    }

    block->used = 0;

    return 0;
}
int vblock_read(vblock *block, int off, unsigned char *buffer, int size)
{
    if (!block || !buffer || !size) {
        jprintf("invalid args\n");
        return -1;
    } else if ((off + size) > block->size(block)) {
        jprintf("overflow \n");
        return -1;
    }

    for (int idx = 0; idx < block->vpage_amt; ++idx)
    {
        if (!size) { break; }

        vpage *page = block->get_page(block, idx);

        int page_size = page->size(page);
        if (page_size > off) {

            int wsize = page_size - off;
            if (size < wsize) {
                wsize = size;
            }

            page->read(page, off, buffer, wsize);

            buffer += wsize;
            size -= wsize;
            off = 0;

        } else {
            off -= page_size;
        }
    }

    return 0;
}
int vblock_write(vblock *block, int off, const unsigned char *buffer, int size)
{
    if (!block || !buffer || !size) {
        jprintf("invalid args\n");
        return -1;
    } else if ((off + size) > block->size(block)) {
        jprintf("overflow \n");
        return -1;
    }

    for (int idx = 0; idx < block->vpage_amt; ++idx)
    {
        if (!size) { break; }

        vpage *page = block->get_page(block, idx);

        int page_size = page->size(page);
        if (page_size > off) {

            int wsize = page_size - off;
            if (size < wsize) {
                wsize = size;
            }

            page->write(page, off, buffer, wsize);

            buffer += wsize;
            size -= wsize;
            off = 0;

        } else {
            off -= page_size;
        }
    }

    block->used = 1;

    return 0;
}
int vblock_size(vblock *block)
{
    if (!block) { return 0; }

    int total_size = 0;
    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        vpage *page = block->get_page(block, idx);
        total_size += page->size(page);
    }

    return total_size;
}
int vblock_page_size(vblock *block)
{
    if (!block) { return 0; }
    vpage *page = block->get_page(block, 0);
    return page->size(page);
}

int vblock_info(vblock *block, int unit_per_line)
{
    if (!block || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    // int page_cnt = block->vpage_amt;
    // int unit_cnt = block->vpage->vunit_amt;
    int page_cnt = 0;
    int unit_cnt = 0;

    // -------------------------------------------------- ... 
    // | 00000000 - 00000000 | xxxxxxxx xxxxxxxx xxxxxxxx ... 
    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("------------");
    printf("-");
    for (int idx = 0; ; ++idx)
    {
        if (!(idx % unit_per_line)) {
            if (idx) { printf("|"); }
            printf("\n");
            printf("| %08d - %08d | ", idx, (idx + unit_per_line));
        }

        if (block->vpage[page_cnt]->vunit[unit_cnt].used) {
            printf("[■ (%02d,%02d)] ",
                    block->vpage[page_cnt]->vunit[unit_cnt].erase,
                    block->vpage[page_cnt]->vunit[unit_cnt].write);
        } else {
            printf("[□ (%02d,%02d)] ",
                    block->vpage[page_cnt]->vunit[unit_cnt].erase,
                    block->vpage[page_cnt]->vunit[unit_cnt].write);
        }

        if (++unit_cnt == block->vpage[page_cnt]->vunit_amt) {
            unit_cnt = 0;
            page_cnt++;
        }
        if (page_cnt == block->vpage_amt) {
            int left = (unit_per_line - (idx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("            ");
                }
            }
            printf("|\n");
            break;
        }
    }

    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("------------");
    printf("-\n");

    return 0;
}
vblock *vblock_dupc(vblock *block)
{
    if (!block) {
        jprintf("invalid args.\n");
        return NULL;
    }

    vblock *block_new = vblock_new();
    if (!block_new) {
        jprintf("block new failed\n");
        return NULL;
    }

    block_new->used = block->used;
    block_new->vpage_amt = block->vpage_amt;
    block_new->vunit_amt = block->vunit_amt;

    block_new->vpage = (vpage **)malloc(sizeof(vpage *) * block_new->vpage_amt);
    if (!block_new->vpage) {
        jprintf("block page point allocate failed\n");
        block_new->destroy(block_new);
        return NULL;
    }
    memset(block_new->vpage, 0, sizeof(vpage *) * block_new->vpage_amt);

    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        block_new->vpage[idx] = block->vpage[idx]->dupc(block->vpage[idx]);
        if (!block_new->vpage[idx]) {
            jprintf("page %d dupc failed\n", idx);
            block_new->destroy(block_new);
            return NULL;
        }
    }

    return block_new;
}
int vblock_wpage_rdump(vblock *block, int wpage_num, vpage *wpage, int unit_per_line)
{
    if (!block || !wpage || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        vpage *page = block->get_page(block, page_num);

        vunit *unit = page->vunit;

        if (page_num == wpage_num) {

            // show write page_num format
            // ---------------------------
            // | 00000001 < FFFFFFFF ... |
            // |          > AFF003E7 ... |
            // ---------------------------

            if (wpage_num) {
                printf("------------");
                for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
                printf("-\n");
            }

            // '<' original data first 
            printf("| %08d < ", page_num);

            int unit_num = 0;
            while(unit_num < page->vunit_amt) {

                // printf("%08X ", unit[unit_num].data);
                VUNIT_TYPE data = unit[unit_num].data;
                unsigned char *shw = (unsigned char *)&data;
                shw += (sizeof(VUNIT_TYPE) - 1);
                for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
                    if ((*shw > ' ') && (*shw < '~')) {
                        printf(".%c", *shw);
                    } else {
                        printf("%02X", *shw);
                    }
                    shw--;
                }
                printf(" ");


                ++unit_num;
                if (unit_num && !(unit_num % unit_per_line)) {
                    printf("|\n");
                    if (unit_num < page->vunit_amt) {
                        printf("|          < ");
                    }
                }
            }
            if (unit_num % unit_per_line) {
                int left = (unit_per_line - (unit_num % unit_per_line));
                if (left) {
                    for (int fill = 0; fill < left; ++fill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }

            // '>' write data first 
            printf("|          > ");

            int wunit_num = 0;
            vunit *wunit = wpage->vunit;
            while(wunit_num < wpage->vunit_amt) {

                // printf("%08X ", wunit[wunit_num].data);
                VUNIT_TYPE data = wunit[wunit_num].data;
                unsigned char *shw = (unsigned char *)&data;
                shw += (sizeof(VUNIT_TYPE) - 1);
                for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
                    if ((*shw > ' ') && (*shw < '~')) {
                        printf(".%c", *shw);
                    } else {
                        printf("%02X", *shw);
                    }
                    shw--;
                }
                printf(" ");

                ++wunit_num;
                if (wunit_num && !(wunit_num % unit_per_line)) {
                    printf("|\n");
                    if (wunit_num < wpage->vunit_amt) {
                        printf("|          > ");
                    }
                }
            }

            if (wunit_num % unit_per_line) {
                int wleft = (unit_per_line - (wunit_num % unit_per_line));
                if (wleft) {
                    for (int wfill = 0; wfill < wleft; ++wfill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }

            if (block->vpage_amt != (page_num + 1)) {
                printf("------------");
                for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
                printf("-\n");
            }

        } else {

            // just same to dump format
            // | 00000001 | FFFFFFFF ... |
            // | 00000002 | AFF003E7 ... |

            printf("| %08d | ", page_num);

            int unit_num = 0;
            while(unit_num < page->vunit_amt) {

                // printf("%08X ", unit[unit_num].data);
                VUNIT_TYPE data = unit[unit_num].data;
                unsigned char *shw = (unsigned char *)&data;
                shw += (sizeof(VUNIT_TYPE) - 1);
                for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
                    if ((*shw > ' ') && (*shw < '~')) {
                        printf(".%c", *shw);
                    } else {
                        printf("%02X", *shw);
                    }
                    shw--;
                }
                printf(" ");

                ++unit_num;
                if (unit_num && !(unit_num % unit_per_line)) {
                    printf("|\n");
                    if (unit_num < page->vunit_amt) {
                        printf("|          | ");
                    }
                }
            }
            if (unit_num % unit_per_line) {
                int left = (unit_per_line - (unit_num % unit_per_line));
                if (left) {
                    for (int wfill = 0; wfill < left; ++wfill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }
        }
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vblock_wpage_dump(vblock *block, int wpage_num, vpage *wpage, int unit_per_line)
{
    if (!block || !wpage || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        vpage *page = block->get_page(block, page_num);

        vunit *unit = page->vunit;

        if (page_num == wpage_num) {

            // show write page_num format
            // ---------------------------
            // | 00000001 < FFFFFFFF ... |
            // |          > AFF003E7 ... |
            // ---------------------------

            if (wpage_num) {
                printf("------------");
                for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
                printf("-\n");
            }

            // original data first 
            printf("| %08d < ", page_num);

            int unit_num = 0;
            while(unit_num < page->vunit_amt) {
                printf("%08X ", unit[unit_num].data);
                ++unit_num;
                if (unit_num && !(unit_num % unit_per_line)) {
                    printf("|\n");
                    if (unit_num < page->vunit_amt) {
                        printf("|          < ");
                    }
                }
            }
            if (unit_num % unit_per_line) {
                int left = (unit_per_line - (unit_num % unit_per_line));
                if (left) {
                    for (int fill = 0; fill < left; ++fill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }

            printf("|          > ");

            // write data first 
            int wunit_num = 0;
            vunit *wunit = wpage->vunit;
            while(wunit_num < wpage->vunit_amt) {
                printf("%08X ", wunit[wunit_num].data);
                ++wunit_num;
                if (wunit_num && !(wunit_num % unit_per_line)) {
                    printf("|\n");
                    if (wunit_num < wpage->vunit_amt) {
                        printf("|          > ");
                    }
                }
            }

            if (wunit_num % unit_per_line) {
                int wleft = (unit_per_line - (wunit_num % unit_per_line));
                if (wleft) {
                    for (int wfill = 0; wfill < wleft; ++wfill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }

            if (block->vpage_amt != (page_num + 1)) {
                printf("------------");
                for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
                printf("-\n");
            }

        } else {

            // just same to dump format
            // | 00000001 | FFFFFFFF ... |
            // | 00000002 | AFF003E7 ... |

            printf("| %08d | ", page_num);

            int unit_num = 0;
            while(unit_num < page->vunit_amt) {
                printf("%08X ", unit[unit_num].data);
                ++unit_num;
                if (unit_num && !(unit_num % unit_per_line)) {
                    printf("|\n");
                    if (unit_num < page->vunit_amt) {
                        printf("|          | ");
                    }
                }
            }
            if (unit_num % unit_per_line) {
                int left = (unit_per_line - (unit_num % unit_per_line));
                if (left) {
                    for (int wfill = 0; wfill < left; ++wfill) {
                        printf("         ");
                    }
                }
                printf("|\n");
            }
        }
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vblock_dump(vblock *block, int unit_per_line)
{
    if (!block || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        vpage *page = block->get_page(block, page_num);
        vunit *unit = page->vunit;

        // just same to dump format
        // | 00000001 | FFFFFFFF ... |
        // | 00000002 | AFF003E7 ... |

        printf("| %08d | ", page_num);

        int unit_num = 0;
        while(unit_num < page->vunit_amt) {
            printf("%08X ", unit[unit_num].data);
            ++unit_num;
            if (unit_num && !(unit_num % unit_per_line)) {
                printf("|\n");
                if (unit_num < page->vunit_amt) {
                    printf("|          | ");
                }
            }
        }
        if (unit_num % unit_per_line) {
            int left = (unit_per_line - (unit_num % unit_per_line));
            if (left) {
                for (int wfill = 0; wfill < left; ++wfill) {
                    printf("         ");
                }
            }
            printf("|\n");
        }
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vblock_dump_read(vblock *block, int unit_per_line)
{
    if (!block || !unit_per_line) {
        jprintf("invaild args.\n");
        return -1;
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        vpage *page = block->get_page(block, page_num);
        vunit *unit = page->vunit;

        // just same to dump format
        // | 00000001 | FFFFFFFF ... |
        // | 00000002 | AFF003E7 ... |

        printf("| %08d | ", page_num);

        int unit_num = 0;
        while(unit_num < page->vunit_amt) {

            // printf("%08X ", unit[unit_num].data);
            VUNIT_TYPE data = unit[unit_num].data;
            unsigned char *shw = (unsigned char *)&data;
            shw += (sizeof(VUNIT_TYPE) - 1);
            for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
                if ((*shw > ' ') && (*shw < '~')) {
                    printf(".%c", *shw);
                } else {
                    printf("%02X", *shw);
                }
                shw--;
            }
            printf(" ");

            ++unit_num;
            if (unit_num && !(unit_num % unit_per_line)) {
                printf("|\n");
                if (unit_num < page->vunit_amt) {
                    printf("|          | ");
                }
            }
        }
        if (unit_num % unit_per_line) {
            int left = (unit_per_line - (unit_num % unit_per_line));
            if (left) {
                for (int wfill = 0; wfill < left; ++wfill) {
                    printf("         ");
                }
            }
            printf("|\n");
        }
    }

    printf("------------");
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    printf("-\n");

    return 0;
}
int vblock_init(vblock *block, int vpage_amt, int vunit_amt)
{
    if (!block || !vpage_amt || !vunit_amt) {
        jprintf("invaild args.\n");
        return -1;
    } else if (block->vpage) {
        jprintf("block page already exist.\n");
        return -1;
    }

    block->used = 0;
    block->vpage_amt = vpage_amt;
    block->vunit_amt = vunit_amt;

    block->vpage = (vpage **)malloc(sizeof(vpage *) * block->vpage_amt);
    if (!block->vpage) {
        jprintf("block page point allocate failed\n");
        return -1;
    }
    memset(block->vpage, 0, sizeof(vpage *) * block->vpage_amt);

    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        vpage *page = vpage_new();
        if (!page) {
            jprintf("page new failed\n");
            return -1;
        } else if (page->init(page, block->vunit_amt)) {
            jprintf("page inint failed\n");
            return -1;
        }
         
        block->vpage[idx] = page;
    }

    return 0;
}
int vblock_destroy(vblock *block)
{
    if (!block) {
        jprintf("invaild args.\n");
        return -1;
    } else if (!block->vpage_amt) {
        return 0;
    }

    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        if (!block->vpage[idx]) { continue; }
        block->vpage[idx]->destroy(block->vpage[idx]);
        block->vpage[idx] = NULL;
    }

    free(block->vpage);
    free(block);

    return 0;
}
vblock *vblock_new(void)
{
    vblock *block = (vblock *)malloc(sizeof(vblock));
    if (!block) {
        jprintf("allocate failed\n");
        return NULL;
    }
    memset(block, 0, sizeof(vblock));

    // assign block method
    block->init = vblock_init;
    block->destroy = vblock_destroy;

    block->dupc = vblock_dupc;

    block->dump = vblock_dump;
    block->rdump = vblock_dump_read;
    block->wpage_dump = vblock_wpage_dump;
    block->wpage_rdump = vblock_wpage_rdump;
    block->info = vblock_info;

    block->get_page = vblock_get_page;

    block->size = vblock_size;
    block->page_size = vblock_page_size;

    block->erase = vblock_erase;
    block->write = vblock_write;
    block->read = vblock_read;

    return block;
}


// ---- vflash ----

// ---- new feature 2022 08 30
int vflash_config_import(vflash *flash, char *name)
{
    if (!flash) {
        jprintf("invaild args.\n");
        return -1;
    }

    if (access(name, F_OK)) {
        jprintf("file \"%s\" not exist.\n", name);
        return -1;
    }

    if (flash->vblock) {
        jprintf("flash was initial.\n");
        return -1;
    }

    // check config size

    // open config file
    FILE *fd = fopen(name, "r");

    int head_size = sizeof(unsigned int) * 3;

    if (!fd) {
        jprintf("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_END)) {
        jprintf("fseek to end failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    } else if (ftell(fd) < head_size) {
        jprintf("config without amount info.\n");
        fclose(fd);
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        jprintf("fseek to start failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    // read first 16 Byte
    unsigned int amount[3] = {0};

    if (fread(amount, sizeof(unsigned int), 3, fd) != 3) {
        jprintf("fread failed : %s.\n", strerror(errno));
        jprintf("amount : %d.%d.%d.\n", amount[0], amount[1], amount[2]);
        fclose(fd);
        return -1;
    }
    jprintf("amount : %d.%d.%d.\n", amount[0], amount[1], amount[2]);

    if (flash->init(flash, amount[0], amount[1], amount[2])) {
        jprintf("flash init failed .\n");
        fclose(fd);
        return -1;
    }

    long total_size = head_size;
    total_size += flash->size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit);

    // have not vunit info is ok , just init to 0
    if (fseek(fd, 0, SEEK_END)) {
        jprintf("fseek to end failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    } else if (ftell(fd) != total_size) {
        jprintf("flash total size mismatch.\n");
        jprintf("ftell (%ld) != total_size (%ld).\n", ftell(fd), total_size);
        fclose(fd);
        return -1;
    } else if (fseek(fd, head_size, SEEK_SET)) {
        jprintf("fseek to head size failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    // read all vunit info from file
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->vblock[bidx];

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->vpage[pidx];

            if (fread(page->vunit, sizeof(vunit), page->vunit_amt, fd) != page->vunit_amt) {
                jprintf("fread failed : %s.\n", strerror(errno));
                jprintf("block %d , page %d : %s.\n", bidx, pidx, strerror(errno));
                return -1;
            }
        }
    }

    fclose(fd);

    memset(flash->file_name, 0, 64);
    snprintf(flash->file_name, 64, "%s", name);

    return 0;
}
int vflash_config_export(vflash *flash, char *name)
{
    if (!flash) {
        jprintf("invalid args\n");
        return -1;
    }

    FILE *fd = fopen(name, "wb");
    if (!fd) {
        jprintf("fopen %s failed : %s\n", name, strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        jprintf("fseek %s failed : %s\n", name, strerror(errno));
        return -1;
    }

    // write vflash config (attribute)
    unsigned int amount[3] = {
        flash->vblock_amt,
        flash->vpage_amt,
        flash->vunit_amt
    };

    if (fwrite(amount, sizeof(unsigned int), 3, fd) != 3) {
        jprintf("fwrite failed : %s.\n", strerror(errno));
        return -1;
    }

    // write all vunit current info
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->vblock[bidx];

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->vpage[pidx];

            if (fwrite(page->vunit, sizeof(vunit), page->vunit_amt, fd) != page->vunit_amt) {
                jprintf("fwrite failed : %s.\n", strerror(errno));
            }
        }
    }

    fclose(fd);

    memset(flash->file_name, 0, 64);
    snprintf(flash->file_name, 64, "%s", name);

    return 0;
}

int vflash_config_page_update(vflash *flash, int block, int page)
{
    if (!flash) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if (block >= flash->vblock_amt) {
        PRINT_ERR("invalid block number\n");
        return -1;
    } else if (page >= flash->vpage_amt) {
        PRINT_ERR("invalid page number\n");
        return -1;
    } else if (!strlen(flash->file_name)) {
        PRINT_ERR("invalid file name, please import or export first.\n");
        return -1;
    }

    long head_off = sizeof(unsigned int) * 3;
    long block_off = head_off + (block * flash->block_size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit));
    long page_off = block_off + (page * flash->page_size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit));

    FILE *fd = fopen(flash->file_name, "r+");
    if (!fd) {
        PRINT_ERR("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, page_off, SEEK_SET)) {
        PRINT_ERR("fseek failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    vblock *wblock = flash->get_block(flash, block);
    if (!wblock) {
        PRINT_ERR("flash get block (%d) failed.\n", block);
        fclose(fd);
        return -1;
    }

    // printf("update block %d , page %d [%X] , ftell %ld.\n", block, page, ftell(fd), ftell(fd));
    vpage *wpage = wblock->get_page(wblock, page);

    if (fwrite(wpage->vunit, sizeof(vunit), wpage->vunit_amt, fd) != wpage->vunit_amt) {
        PRINT_ERR("fwrite failed : %s.\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    fclose(fd);

    return 0;
}

int vflash_config_block_update(vflash *flash, int block)
{
    if (!flash) {
        jprintf("invalid args\n");
        return -1;
    } else if (block >= flash->vblock_amt) {
        jprintf("invalid block number\n");
        return -1;
    } else if (!strlen(flash->file_name)) {
        jprintf("invalid file name, please import or export first.\n");
        return -1;
    }

    long head_off = sizeof(unsigned int) * 3;
    long block_off = head_off + (block * flash->block_size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit));

    FILE *fd = fopen(flash->file_name, "r+");
    if (!fd) {
        jprintf("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, block_off, SEEK_SET)) {
        jprintf("fseek failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    vblock *wblock = flash->get_block(flash, block);
    if (!wblock) {
        jprintf("flash get block (%d) failed.\n", block);
        fclose(fd);
        return -1;
    }

    // printf("update block %d , ftell %ld.\n", block, ftell(fd));
    for (int pidx = 0; pidx < wblock->vpage_amt; ++pidx) {

        vpage *page = wblock->vpage[pidx];

        if (fwrite(page->vunit, sizeof(vunit), page->vunit_amt, fd) != page->vunit_amt) {
            jprintf("fwrite failed : %s.\n", strerror(errno));
            fclose(fd);
            return -1;
        }
    }

    fclose(fd);

    return 0;
}

// ---- new feature 2022 08 23
int vflash_file_dump(vflash *flash)
{
    if (!flash) {
        jprintf("invalid args\n");
        return -1;
    }

    FILE *fd = fopen("flash.instance", "wb");
    if (!fd) {
        jprintf("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        jprintf("fseek failed : %s\n", strerror(errno));
        return -1;
    }

    int block_size = flash->block_size(flash);
    unsigned char *wdata = (unsigned char *)malloc(block_size);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        memset(wdata, 0, block_size);
        vblock *block = flash->vblock[bidx];
        if (block->read(block, 0, wdata, block_size)) {
            jprintf("block %d read failed.\n", bidx);
            continue;
        } else if (fwrite(wdata, block_size, 1, fd) < 0) {
            jprintf("fwrite failed : %s.\n", strerror(errno));
            continue;
        }

    }

    free(wdata);

    fclose(fd);

    return 0;
}

vblock *vflash_get_block(vflash *flash, int block_num)
{
    if (!flash || (block_num >= flash->vblock_amt)) {
        jprintf("invalid args\n");
        return NULL;
    } else if (!flash->vblock) {
        jprintf("no init.\n");
        return NULL;
    }

    return flash->vblock[block_num];
}

int vflash_size(vflash *flash)
{
    if (!flash) { return 0; }

    int total_size = 0;
    for (int idx = 0; idx < flash->vblock_amt; ++idx) {
        if (!flash->vblock[idx]) {
            printf("[WARNING] WTF !! flash block leak ...\n");
            continue;
        }
        total_size += flash->vblock[idx]->size(flash->vblock[idx]);
    }

    return total_size;
}
int vflash_block_size(vflash *flash)
{
    if (!flash) { return 0; }

    vblock *block = flash->get_block(flash, 0);

    return block->size(block);
}
int vflash_unit_size(vflash *flash)
{
    if (!flash) { return 0; }
    return sizeof(VUNIT_TYPE);
}
int vflash_page_size(vflash *flash)
{
    if (!flash) { return 0; }
    vblock *block = flash->get_block(flash, 0);
    vpage *page = block->get_page(block, 0);
    return page->size(page);
}
int vflash_file_load(vflash *flash, char *file)
{
    if (!flash || !file) {
        jprintf("invaild args.\n");
        return -1;
    }

    if (access(file, F_OK)) {
        jprintf("file not exist.\n");
        return -1;
    }

    // open config file
    FILE *fd = fopen("flash.instance", "r");
    int load_result = 0;
    if (!fd) {
        jprintf("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_END)) {
        jprintf("fseek to end failed : %s\n", strerror(errno));
        load_result = -1;
    } else if (ftell(fd) != flash->size(flash)) {
        jprintf("file size mismatch.\n");
        load_result = -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        jprintf("fseek to start failed : %s\n", strerror(errno));
        load_result = -1;
    } else {

        int block_size = flash->block_size(flash);
        unsigned char *rdata = (unsigned char *)malloc(block_size);

        for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
            memset(rdata, 0, block_size);
            vblock *block = flash->vblock[bidx];

            if (fread(rdata, block_size, 1, fd) < 0) {
                jprintf("fread failed : %s.\n", strerror(errno));
                load_result = -1;
                break;
            } else if (block->write(block, 0, rdata, block_size)) {
                jprintf("block %d read failed.\n", bidx);
                load_result = -1;
                break;
            }
        }

        free(rdata);
    }

    fclose(fd);

    return load_result;
}
int vflash_init(vflash *flash, int vblock_amt, int vpage_amt, int vunit_amt)
{
    if (!flash || !vblock_amt || !vpage_amt || !vunit_amt) {
        jprintf("invaild args.\n");
        return -1;
    }

    flash->used = 0;
    flash->vblock_amt = vblock_amt;
    flash->vpage_amt = vpage_amt;
    flash->vunit_amt = vunit_amt;

    flash->vblock = (vblock **)malloc(sizeof(vblock *) * flash->vblock_amt);
    if (!flash->vblock) {
        jprintf("vblock point array allocate failed.\n");
        return -1;
    }
    memset(flash->vblock, 0, sizeof(vblock *) * flash->vblock_amt);

    for (int idx = 0; idx < flash->vblock_amt; ++idx) {
        vblock *block = vblock_new();
        if (!block) {
            jprintf("vblock %d new failed.\n", idx);
            return -1;
        } else if (block->init(block, flash->vpage_amt, flash->vunit_amt)) {
            jprintf("vblock %d init failed.\n", idx);
            block->destroy(block);
            return -1;
        }

        flash->vblock[idx] = block;
    }

    return 0;
}

vflash *vflash_dupc(vflash *flash)
{
    if (!flash) {
        jprintf("invliad args.\n");
        return NULL;
    }

    vflash *flash_new = vflash_new();
    if (!flash_new) {
        jprintf("flash new failed\n");
    }

    flash_new->used       = flash->used;
    flash_new->vblock_amt = flash->vblock_amt;
    flash_new->vpage_amt  = flash->vpage_amt;
    flash_new->vunit_amt  = flash->vunit_amt;

    flash_new->vblock = (vblock **)malloc(sizeof(vblock *) * flash_new->vblock_amt);
    if (!flash_new->vblock) {
        jprintf("vblock point array allocate failed.\n");
        flash_new->destroy(flash_new);
        return NULL;
    }
    memset(flash_new->vblock, 0, sizeof(vblock *) * flash_new->vblock_amt);

    for (int idx = 0; idx < flash_new->vblock_amt; ++idx) {
        flash_new->vblock[idx] = flash->vblock[idx]->dupc(flash->vblock[idx]);
        if (!flash_new->vblock[idx]) {
            jprintf("block %d dupc failed\n", idx);
            flash_new->destroy(flash_new);
            return NULL;
        }
    }

    return flash_new;
}

int vflash_destroy(vflash *flash)
{
    if (!flash) { return 0; }

    if (!flash->vblock_amt) { return 0; }

    if (flash->vblock) {
        for (int idx = 0; idx < flash->vblock_amt; ++idx) {
            if (!flash->vblock[idx]) { continue; }
            flash->vblock[idx]->destroy(flash->vblock[idx]);
            flash->vblock[idx] = NULL;
        }
    }

    free(flash->vblock);

    free(flash);

    return 0;
}

vflash *vflash_new(void)
{
    vflash *flash = (vflash *)malloc(sizeof(vflash));
    if (!flash) {
        jprintf("allocate failed\n");
        return NULL;
    }
    memset(flash, 0, sizeof(vflash));

    flash->init = vflash_init;
    flash->destroy = vflash_destroy;

    flash->dupc = vflash_dupc;

    flash->size = vflash_size;
    flash->block_size = vflash_block_size;
    flash->page_size = vflash_page_size;
    flash->unit_size = vflash_unit_size;

    flash->get_block = vflash_get_block;

    // new feature 2022 08 23
    flash->fdump = vflash_file_dump;
    flash->fload = vflash_file_load;

    // new feature 2022 08 30
    flash->fimport = vflash_config_import;
    flash->fexport = vflash_config_export;
    flash->fblock_update = vflash_config_block_update;
    flash->fpage_update = vflash_config_page_update;

    return flash;
}
