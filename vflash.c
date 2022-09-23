#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "misc.h"
#include "vflash.h"

static void number_line_bar(int unit_per_line)
{
    // ---------------------------------------------
    // | 00000001 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    // ------------
    printf("------------");
    // ---------------------------------------------
    // | 00000001 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    //             ---------
    //                      ---------
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    // ---------------------------------------------
    // | 00000001 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    //                                            --
    printf("--\n");
}
static void number_info_line_bar(int unit_per_line, int align)
{
    // ---------------------------------------------
    // | 00000000 | [□ (xx,oo)] [■ (xx,oo)] [□ (xx,oo)] ... 
    // -------------
    printf("-------------");

    for (int idx = 0; idx < unit_per_line; ++idx) {
    // ---------------------------------------------
    // | 00000000 | [□ (xx,oo)] [■ (xx,oo)] [□ (xx,oo)] ... 
    //              ----
        printf("----");

    // | 00000000 | [□ (??,??)] [■ (???,???)] [■ (xx,oo)] ... 
    //                  -----  or   -------
        for (int an = 0; an < (align * 2 + 1); ++an) printf("-");

    // | 00000000 | [■ (??,??)] [■ (???,???)] [■ (xx,oo)] ... 
    //                       ---
        printf("---");
    }
    // | 00000000 | [■ (??,??)] [■ (???,???)] [■ (xx,oo)] |
    //                                                    -
    printf("-\n");
}
static void range_line_bar(int unit_per_line)
{
    // ---------------------------------------------
    // | 00000000 - 00000000 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    // -----------------------
    printf("-----------------------");
    // ---------------------------------------------
    // | 00000000 - 00000000 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    //                        ---------
    //                                 ---------
    for (int idx = 0; idx < unit_per_line; ++idx) printf("---------");
    // ---------------------------------------------
    // | 00000000 - 00000000 | FFFFFFFF FFFFFFFF ... FFFFFFFF |
    //                                                       --
    printf("--\n");
}
static void range_info_line_bar(int unit_per_line, int align)
{
    // ------------------------
    // | 00000000 - 00000000 | [■ (xx,oo)] [■ (xx,oo)] [■ (xx,oo)] ... 
    printf("------------------------");
    for (int idx = 0; idx < unit_per_line; ++idx) {
        //                         ----
        // | 00000000 - 00000000 | [■ (xx,oo)] [■ (xx,oo)] [■ (xx,oo)] ... 
        printf("----");
        //                             -----       -------
        // | 00000000 - 00000000 | [■ (??,??)] [■ (???,???)] [■ (xx,oo)] ... 
        for (int an = 0; an < (align * 2 + 1); ++an) printf("-");

        //                                  ---
        // | 00000000 - 00000000 | [■ (xx,oo)] [■ (xx,oo)] [■ (xx,oo)] ... 
        printf("---");
    }
    printf("-\n");

    return;
}

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

int vpage_copy(vpage *page, vpage *src_page)
{
    if (!page || !src_page) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (page->size(page) != src_page->size(src_page)) {
        PRINT_ERR("%s() page size mismatch.\n", __func__);
        return -1;
    }

    page->erase(page);

    for (int idx = 0; idx < page->vunit_amt; ++idx) {

        if (!src_page->vunit[idx].used) { continue; }

        if (page->vunit[idx].used) {
            PRINT_ERR("page unit %d over write.\n", idx);
            return -1;
        }

        page->vunit[idx].used = src_page->vunit[idx].used;
        page->vunit[idx].data = src_page->vunit[idx].data;
    }

    return 0;
}
int vpage_copy_without(vpage *page, vpage *src_page, int off, int size)
{
    if (!page || !src_page) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (page->size(page) != src_page->size(src_page)) {
        PRINT_ERR("%s() page size mismatch.\n", __func__);
        return -1;
    } else if (off + size > page->size(page)) {
        PRINT_ERR("%s() invalid offset or size.\n", __func__);
        return -1;
    }

    int align = sizeof(VUNIT_TYPE);
    if (off % align) {
        PRINT_ERR("%s() offset not align vunit size (%d).\n", __func__, align);
        return -1;
    } else if (size % align) {
        PRINT_ERR("%s() size not align vunit size (%d).\n", __func__, align);
        return -1;
    }

    int skip_unit = off / sizeof(VUNIT_TYPE);
    int skip_size = size / sizeof(VUNIT_TYPE);

    page->erase(page);

    for (int idx = 0; idx < page->vunit_amt; ++idx) {

        if (idx >= skip_unit && idx <= (skip_unit + skip_size)) {
            printf("page copy skip : unit %d.\n", idx);
            continue;
        }

        if (!src_page->vunit[idx].used) { continue; }

        if (page->vunit[idx].used) {
            PRINT_ERR("%s() vunit %d in used , over write.\n", __func__, idx);
            return -1;
        }

        page->vunit[idx].used = src_page->vunit[idx].used;
        page->vunit[idx].data = src_page->vunit[idx].data;
    }

    return 0;
}
vpage *vpage_dupc(vpage *page)
{
    if (!page) {
        PRINT_ERR("invaild args.\n");
        return NULL;
    }

    vpage *page_new = vpage_new();
    if (!page_new) {
        PRINT_ERR("page new failed\n");
        return NULL;
    } else if (page_new->init(page_new, page->vunit_amt)) {
        PRINT_ERR("page init failed\n");
        page_new->destroy(page_new);
        return NULL;
    }

    memcpy(page->vunit, page_new->vunit, sizeof(vunit) * page->vunit_amt);

    return page_new;
}
int vpage_dump(vpage *page, int unit_per_line)
{
    if (!page || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    int unit_cnt = 0;

    // -------------------------------------------------- ... 
    // | 00000000 - 00000000 | xxxxxxxx xxxxxxxx xxxxxxxx ... 
    range_line_bar(unit_per_line);
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

    range_line_bar(unit_per_line);

    return 0;
}
int vpage_dump_read(vpage *page, int unit_per_line)
{
    if (!page || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    int unit_cnt = 0;

    // -------------------------------------------------- ...
    // | block readable dump : [PAGE 0].
    // -------------------------------------------------- ... 
    // | page : oooo unit ( xxxx bytes )
    // -------------------------------------------------- ... 
    // | 00000000 - 00000000 | xxxxxxxx xxxxxxxx xxxxxxxx ... 
    range_line_bar(unit_per_line);
    printf("| page : %4d unit ( %d bytes )\n", page->vunit_amt, page->size(page));
    range_line_bar(unit_per_line);

    for (int uidx = 0; uidx < page->vunit_amt; ++uidx) {

        // | 00000000 - 00000000 |  
        if (!(uidx % unit_per_line)) {
            if (uidx) { printf("|\n"); }
            printf("| %08d - %08d | ", uidx, (uidx + unit_per_line));
        }

        //                         xxxxxxxx xxxxxxxx xxxxxxxx ... 
        VUNIT_TYPE data = page->vunit[uidx].data;
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

        if ((uidx + 1) == page->vunit_amt) {
            int left = (unit_per_line - (uidx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("         ");
                }
            }
            printf("|\n");
        }
    }

    range_line_bar(unit_per_line);

    return 0;
}
int vpage_info(vpage *page, int unit_per_line, int align)
{
    if (!page || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    // ---------------------------------------------------------------------------
    // | page num : oooooooo  | ■ : in used , □ : unused  
    // ---------------------------------------------------------------------------
    // | [■] page : oooo unit | %d bytes in total
    // ---------------------------------------------------------------------------
    range_info_line_bar(unit_per_line, align);
    if (page->used) {
        printf("| [■] page : %4d unit  | %d bytes in total\n", page->vunit_amt, page->size(page));
    } else {
        printf("| [□] page : %4d unit  | %d bytes in total\n", page->vunit_amt, page->size(page));
    }
    range_info_line_bar(unit_per_line, align);

    char ufmt[32] = {0};
    snprintf(ufmt, 32, "[■ (%%%dd,%%%dd)] ", align, align);

    char nfmt[32] = {0};
    snprintf(nfmt, 32, "[□ (%%%dd,%%%dd)] ", align, align);

    for (int idx = 0; idx < page->vunit_amt; ++idx) {

        if (!(idx % unit_per_line)) {
            if (idx) { printf("|\n"); }
            printf("| %08d - %08d | ", idx, (idx + unit_per_line));
        }

        if (page->vunit[idx].used) {
            printf(ufmt, page->vunit[idx].erase, page->vunit[idx].write);
        } else {
            printf(nfmt, page->vunit[idx].erase, page->vunit[idx].write);
        }

        if ((idx + 1) == page->vunit_amt) {
            int left = (unit_per_line - (idx % unit_per_line) - 1);
            if (left) {
                for (int fill = 0; fill < left; ++fill) {
                    printf("    ");
                    for (int an = 0; an < (align * 2 + 1); ++an) printf(" ");
                    printf("   ");
                }
            }
            printf("|\n");
        }
    }
    range_info_line_bar(unit_per_line, align);
    printf("\n");

    return 0;
}

int vpage_erase(vpage *page)
{
    if (!page) {
        PRINT_ERR("invalid args\n");
        return -1;
    }

    for (int idx = 0; idx < page->vunit_amt; ++idx) {
        page->vunit[idx].used = 0;
        page->vunit[idx].erase++;
        page->vunit[idx].data = (VUNIT_TYPE)-1;
    }

    page->used = 0;

    return 0;
}
int vpage_read(vpage *page, int off, unsigned char *buffer, int size)
{
    if (!page || !buffer || !size) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if ((off + size) > page->size(page)) {
        PRINT_ERR("overflow \n");
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
        for (int idx = 0; idx < no_align; ++idx) {
            buffer[idx] = page->vunit[unit_index].data >> ((align - no_align + idx) * 8);
        }
        buffer += no_align;
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
int vpage_write(vpage *page, unsigned int off, const unsigned char *buffer, unsigned int size)
{
    if (!page || !buffer || !size) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if ((off + size) > page->size(page)) {
        PRINT_ERR("overflow \n");
        return -1;
    }

    //  <--           page size           -->
    // | ...  |    |    |    |    |    | ... |
    //      ->      <-     ■■ □□□□ □□□□  ...
    //       unit size      ^   ^ align data
    //                    not align data

    int align = sizeof(VUNIT_TYPE);
    if (off % align) {
        printf("off no align.\n");
        int no_align = align - (off % align);
        int unit_index = off / align;
        if (page->vunit[unit_index].used) {
            PRINT_ERR("over write.\n");
            return -1;
        }
        page->vunit[unit_index].write++;
        page->vunit[unit_index].used = 1;
        for (int idx = 0; idx < no_align; ++idx) {
            page->vunit[unit_index].data &= ~(0xFF << ((align - no_align + idx) * 8));
            page->vunit[unit_index].data |= (buffer[idx] << ((align - no_align + idx) * 8));
        }
        buffer += no_align;
        off += no_align;
        size -= no_align;
    }

    //  <--           page size           -->
    // | ...  |    |   ...   |    |    | ... |
    //      ->      <-   □□□□ □□□□ ■■    ...
    //       unit size   ^         ^ not align data
    //                   align data

    if (size % align) {
        printf("size no align.\n");
        int no_align = size % align;
        int unit_index = (off + size) / align;
        if (page->vunit[unit_index].used) {
            PRINT_ERR("over write.\n");
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
        int uidx = (off / align) + idx;
        if (page->vunit[uidx].used) {
            PRINT_ERR("page %d , unit %d : over write.\n", page->seq, uidx);
            return -1;
        }
        if (*vbuff != (VUNIT_TYPE)-1) {
            page->vunit[uidx].write++;
            page->vunit[uidx].used = 1;
            page->vunit[uidx].data = *vbuff;
            vunit_reverse(&(page->vunit[uidx].data));
            page->used = 1;
        }
        vbuff++;
    }

    return 0;
}
VUNIT_TYPE vpage_get_unit(vpage *page, unsigned int unit)
{
    if (!page) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return (VUNIT_TYPE)0;
    } else if (!page->vunit) {
        PRINT_ERR("%s() no init.\n", __func__);
        return (VUNIT_TYPE)0;
    } else if (unit > page->vunit_amt) {
        PRINT_ERR("%s() invalid unit.\n", __func__);
        return (VUNIT_TYPE)0;
    }

    VUNIT_TYPE res = page->vunit[unit].data;

    vunit_reverse(&res);

    return res;
}
int vpage_addr(vpage *page)
{
    if (!page) { return 0; }

    return page->seq * page->size(page);
}
int vpage_size(vpage *page)
{
    if (!page) { return 0; }

    return page->vunit_amt * sizeof(VUNIT_TYPE);
}
int vpage_init(vpage *page, int vunit_amt)
{
    if (!page || !vunit_amt) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    page->used = 0;
    page->vunit_amt = vunit_amt;
    page->vunit = (vunit *)malloc(sizeof(vunit) * vunit_amt);
    if (!page->vunit) {
        PRINT_ERR("page new unit failed\n");
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
        PRINT_ERR("invaild args.\n");
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
        PRINT_ERR("allocate failed\n");
        return NULL;
    }
    memset(page, 0, sizeof(vpage));

    // assign page method
    page->init         = vpage_init;
    page->destroy      = vpage_destroy;
    page->size         = vpage_size;
    page->addr         = vpage_addr;
    page->erase        = vpage_erase;
    page->write        = vpage_write;
    page->read         = vpage_read;
    page->get_unit     = vpage_get_unit;
    page->dump         = vpage_dump;
    page->dupc         = vpage_dupc;
    page->copy         = vpage_copy;
    page->copy_without = vpage_copy_without;
    page->rdump        = vpage_dump_read;
    page->info         = vpage_info;

    return page;
}

// ---- vblock ----

int vblock_calc_page_addr(vblock *block, int page)
{
    if (!block || (page >= block->vpage_amt)) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if (!block->vpage) {
        PRINT_ERR("no init\n");
        return -1;
    }

    return page * block->page_size(block);
}
vpage *vblock_get_page_from_addr(vblock *block, int addr)
{
    if (!block) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return NULL;
    } else if (addr > block->size(block)) {
        PRINT_ERR("%s() invalid addr.\n", __func__);
        return NULL;
    }

    int page = block->size(block) / addr;

    return block->get_page(block, page);
}
vpage *vblock_get_page(vblock *block, int page_num)
{
    if (!block || (page_num >= block->vpage_amt)) {
        PRINT_ERR("invalid args\n");
        return NULL;
    } else if (!block->vpage) {
        PRINT_ERR("no init\n");
        return NULL;
    }

    return block->vpage[page_num];
}
int vblock_erase(vblock *block)
{
    if (!block) {
        PRINT_ERR("invalid args\n");
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
        PRINT_ERR("invalid args\n");
        return -1;
    } else if ((off + size) > block->size(block)) {
        PRINT_ERR("overflow \n");
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
int vblock_write(vblock *block, unsigned int off, const unsigned char *buffer, unsigned int size)
{
    if (!block || !buffer || !size) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if ((off + size) > block->size(block)) {
        PRINT_ERR("block %d , off %d , size %d : overflow \n", block->seq, off, size);
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
int vblock_addr(vblock *block)
{
    if (!block) { return 0; }

    return block->seq * block->size(block);
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

int vblock_info(vblock *block, int unit_per_line, int align)
{
    if (!block || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }


    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        // ---------------------------------------------------------------------------
        // | page num : oooooooo | ■ : in used , □ : unused  
        range_info_line_bar(unit_per_line, align);
        printf("| page num : %08d  | ■ : in used , □ : unused\n", page_num);

        vpage *page = block->get_page(block, page_num);
        page->info(page, unit_per_line, align);

    }

    return 0;
}
vblock *vblock_dupc(vblock *block)
{
    if (!block) {
        PRINT_ERR("invalid args.\n");
        return NULL;
    }

    vblock *block_new = vblock_new();
    if (!block_new) {
        PRINT_ERR("block new failed\n");
        return NULL;
    } else if (block_new->init(block_new, block->vpage_amt, block->vunit_amt)) {
        PRINT_ERR("block new init failed.\n");
        block_new->destroy(block_new);
        return NULL;
    }

    block_new->used = block->used;

    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        vpage *page = block->get_page(block, idx);
        vpage *page_new = block_new->get_page(block_new, idx);
        if (page_new->copy(page_new, page)) {
            PRINT_ERR("%s() page copy failed.\n", __func__);
            block_new->destroy(block_new);
            return NULL;
        }
    }

    return block_new;
}
int vblock_wpage_rdump(vblock *block, int wpage_num, vpage *wpage, int unit_per_line)
{
    if (!block || !wpage || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    number_line_bar(unit_per_line);

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
                number_line_bar(unit_per_line);
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
                number_line_bar(unit_per_line);
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

    number_line_bar(unit_per_line);

    return 0;
}
int vblock_wpage_dump(vblock *block, int wpage_num, vpage *wpage, int unit_per_line)
{
    if (!block || !wpage || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    number_line_bar(unit_per_line);

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
                number_line_bar(unit_per_line);
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
                number_line_bar(unit_per_line);
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

    number_line_bar(unit_per_line);

    return 0;
}

int vblock_dump(vblock *block, int unit_per_line)
{
    if (!block || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    number_line_bar(unit_per_line);

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

    number_line_bar(unit_per_line);

    return 0;
}
int vblock_dump_read(vblock *block, int unit_per_line)
{
    if (!block || !unit_per_line) {
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    for (int page_num = 0; page_num < block->vpage_amt; ++page_num) {

        vpage *page = block->get_page(block, page_num);
        vunit *unit = page->vunit;

        // ---------------------------
        // | block readable dump : [PAGE 0].
        range_line_bar(unit_per_line);
        printf("| block readable dump : [PAGE %d].\n", page_num);
        page->rdump(page, unit_per_line);

//      // just same to dump format
//      // | 00000001 | FFFFFFFF ... |
//      // | 00000002 | AFF003E7 ... |

//      printf("| %08d | ", page_num);

//      int unit_num = 0;
//      while(unit_num < page->vunit_amt) {

//          // printf("%08X ", unit[unit_num].data);
//          VUNIT_TYPE data = unit[unit_num].data;
//          unsigned char *shw = (unsigned char *)&data;
//          shw += (sizeof(VUNIT_TYPE) - 1);
//          for (int sidx = 0; sidx < sizeof(VUNIT_TYPE); ++sidx) {
//              if ((*shw > ' ') && (*shw < '~')) {
//                  printf(".%c", *shw);
//              } else {
//                  printf("%02X", *shw);
//              }
//              shw--;
//          }
//          printf(" ");

//          ++unit_num;
//          if (unit_num && !(unit_num % unit_per_line)) {
//              printf("|\n");
//              if (unit_num < page->vunit_amt) {
//                  printf("|          | ");
//              }
//          }
//      }
//      if (unit_num % unit_per_line) {
//          int left = (unit_per_line - (unit_num % unit_per_line));
//          if (left) {
//              for (int wfill = 0; wfill < left; ++wfill) {
//                  printf("         ");
//              }
//          }
//          printf("|\n");
//      }
    }

    number_line_bar(unit_per_line);

    return 0;
}
int vblock_init(vblock *block, int vpage_amt, int vunit_amt)
{
    if (!block || !vpage_amt || !vunit_amt) {
        PRINT_ERR("invaild args.\n");
        return -1;
    } else if (block->vpage) {
        PRINT_ERR("block page already exist.\n");
        return -1;
    }

    block->used = 0;
    block->vpage_amt = vpage_amt;
    block->vunit_amt = vunit_amt;

    block->vpage = (vpage **)malloc(sizeof(vpage *) * block->vpage_amt);
    if (!block->vpage) {
        PRINT_ERR("block page point allocate failed\n");
        return -1;
    }
    memset(block->vpage, 0, sizeof(vpage *) * block->vpage_amt);

    for (int idx = 0; idx < block->vpage_amt; ++idx) {
        vpage *page = vpage_new();
        if (!page) {
            PRINT_ERR("page new failed\n");
            return -1;
        } else if (page->init(page, block->vunit_amt)) {
            PRINT_ERR("page inint failed\n");
            return -1;
        }
         
        page->seq = idx;
        block->vpage[idx] = page;
        if (idx) {
            block->vpage[idx - 1]->next = page;
        }
    }

    return 0;
}
int vblock_destroy(vblock *block)
{
    if (!block) {
        PRINT_ERR("invaild args.\n");
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
        PRINT_ERR("allocate failed\n");
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
    block->get_page_from_addr = vblock_get_page_from_addr;
    block->calc_page_addr = vblock_calc_page_addr;

    block->size = vblock_size;
    block->addr = vblock_addr;
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
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    if (access(name, F_OK)) {
        PRINT_ERR("file \"%s\" not exist.\n", name);
        return -1;
    }

    if (flash->vblock) {
        PRINT_ERR("flash was initial.\n");
        return -1;
    }

    // check config size

    // open config file
    FILE *fd = fopen(name, "r");

    int head_size = sizeof(unsigned int) * 3;

    if (!fd) {
        PRINT_ERR("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_END)) {
        PRINT_ERR("fseek to end failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    } else if (ftell(fd) < head_size) {
        PRINT_ERR("config without amount info.\n");
        fclose(fd);
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        PRINT_ERR("fseek to start failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    // read first 16 Byte
    unsigned int amount[3] = {0};

    if (fread(amount, sizeof(unsigned int), 3, fd) != 3) {
        PRINT_ERR("fread failed : %s.\n", strerror(errno));
        PRINT_ERR("amount : %d.%d.%d.\n", amount[0], amount[1], amount[2]);
        fclose(fd);
        return -1;
    }

    PRINT("amount : %d(blk/flash) * %d(pag/blk) * %d(unit/pag) * %ld(byte/unit).\n", amount[0], amount[1], amount[2], sizeof(VUNIT_TYPE));

    if (flash->init(flash, amount[0], amount[1], amount[2])) {
        PRINT_ERR("flash init failed .\n");
        fclose(fd);
        return -1;
    }

    long total_size = head_size;
    total_size += flash->size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit);

    // have not vunit info is ok , just init to 0
    if (fseek(fd, 0, SEEK_END)) {
        PRINT_ERR("fseek to end failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    } else if (ftell(fd) != total_size) {
        PRINT_ERR("flash total size mismatch.\n");
        PRINT_ERR("ftell (%ld) != total_size (%ld).\n", ftell(fd), total_size);
        fclose(fd);
        return -1;
    } else if (fseek(fd, head_size, SEEK_SET)) {
        PRINT_ERR("fseek to head size failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    // read all vunit info from file
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->vblock[bidx];

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->vpage[pidx];

            if (fread(page->vunit, sizeof(vunit), page->vunit_amt, fd) != page->vunit_amt) {
                PRINT_ERR("fread failed : %s.\n", strerror(errno));
                PRINT_ERR("block %d , page %d : %s.\n", bidx, pidx, strerror(errno));
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
        PRINT_ERR("invalid args\n");
        return -1;
    }

    FILE *fd = fopen(name, "wb");
    if (!fd) {
        PRINT_ERR("fopen %s failed : %s\n", name, strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        PRINT_ERR("fseek %s failed : %s\n", name, strerror(errno));
        return -1;
    }

    // write vflash config (attribute)
    unsigned int amount[3] = {
        flash->vblock_amt,
        flash->vpage_amt,
        flash->vunit_amt
    };

    if (fwrite(amount, sizeof(unsigned int), 3, fd) != 3) {
        PRINT_ERR("fwrite failed : %s.\n", strerror(errno));
        return -1;
    }

    // write all vunit current info
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->vblock[bidx];

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->vpage[pidx];

            if (fwrite(page->vunit, sizeof(vunit), page->vunit_amt, fd) != page->vunit_amt) {
                PRINT_ERR("fwrite failed : %s.\n", strerror(errno));
            }
        }
    }

    fclose(fd);

    memset(flash->file_name, 0, 64);
    snprintf(flash->file_name, 64, "%s", name);

    return 0;
}

int vflash_check_addr(vflash *flash)
{
    if (!flash) {
        PRINT_ERR("invalid args\n");
        return -1;
    }

    // null check
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
        if (!flash->get_block(flash, bidx)) {
            PRINT_ERR("%s() block %d is NULL.\n", __func__, bidx);
            return -1;
        }
    }

    // same address check
    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
        vblock *block = flash->get_block(flash, bidx);
        for (int cidx = 0; cidx < flash->vblock_amt; ++cidx) {
            if (bidx == cidx) { continue; }
            if (block == flash->get_block(flash, cidx)) {
                PRINT_ERR("%s() block %d and block %d addr is same.\n", __func__, bidx, cidx);
                return -1;
            }
        }
    }

    return 0;
}
int vflash_check_val(vflash *flash)
{
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

static long vflash_config_mapping_block_address(vflash *flash, int block, int off)
{
    if (!flash) {
        PRINT_ERR("invalid args\n");
        return 0;
    }

    long head_off = sizeof(unsigned int) * 3;
    long block_off = head_off + (block * flash->block_size(flash) / sizeof(VUNIT_TYPE) * sizeof(vunit));
    long data_off = block_off + (off / sizeof(VUNIT_TYPE) * sizeof(vunit));

    return data_off;
}
int vflash_config_block_update(vflash *flash, int block, int off, int size)
{
    if (!flash) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if (block >= flash->vblock_amt) {
        PRINT_ERR("invalid block number\n");
        return -1;
    } else if ((off + size) > flash->block_size(flash)) {
        PRINT_ERR("invalid off or size number\n");
        return -1;
    } else if (off % sizeof(VUNIT_TYPE)) {
        PRINT_ERR("offset not align\n");
        return -1;
    } else if (size % sizeof(VUNIT_TYPE)) {
        PRINT_ERR("size not align\n");
        return -1;
    } else if (!strlen(flash->file_name)) {
        PRINT_ERR("invalid file name, please import or export first.\n");
        return -1;
    }

    // jump to specific file address ( xxx.config address )
    long blk_off = vflash_config_mapping_block_address(flash, block, off);

    FILE *fd = fopen(flash->file_name, "r+");
    if (!fd) {
        PRINT_ERR("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, blk_off, SEEK_SET)) {
        PRINT_ERR("fseek failed : %s\n", strerror(errno));
        fclose(fd);
        return -1;
    }

    // find specific page and unit memory address ( RAM address )
    vblock *wblock = flash->get_block(flash, block);
    if (!wblock) {
        PRINT_ERR("flash get block (%d) failed.\n", block);
        fclose(fd);
        return -1;
    }

    int pnum = off / wblock->page_size(wblock);
    int unum = (off % wblock->page_size(wblock)) / sizeof(VUNIT_TYPE);

    // write unit one by one
    long update_unit_numbers = size / sizeof(VUNIT_TYPE);
    while(update_unit_numbers--) {

        vpage *wpage = wblock->get_page(wblock, pnum);
        vunit *wunit = &wpage->vunit[unum];

        if (fwrite(wunit, sizeof(vunit), 1, fd) != 1) {
            PRINT_ERR("fwrite failed : %s.\n", strerror(errno));
            fclose(fd);
            return -1;
        }

        unum++;

        if (unum == wpage->vunit_amt) {
            pnum++;
            unum = 0;
        }
    }

    fclose(fd);

    return 0;
}

// ---- new feature 2022 08 23
int vflash_file_dump(vflash *flash)
{
    if (!flash) {
        PRINT_ERR("invalid args\n");
        return -1;
    }

    FILE *fd = fopen("flash.instance", "wb");
    if (!fd) {
        PRINT_ERR("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        PRINT_ERR("fseek failed : %s\n", strerror(errno));
        return -1;
    }

    int block_size = flash->block_size(flash);
    unsigned char *wdata = (unsigned char *)malloc(block_size);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        memset(wdata, 0, block_size);
        vblock *block = flash->vblock[bidx];
        if (block->read(block, 0, wdata, block_size)) {
            PRINT_ERR("block %d read failed.\n", bidx);
            continue;
        } else if (fwrite(wdata, block_size, 1, fd) < 0) {
            PRINT_ERR("fwrite failed : %s.\n", strerror(errno));
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
        PRINT_ERR("invalid args , flash(%p) || get block(%d)\n", flash, block_num);
        return NULL;
    } else if (!flash->vblock) {
        PRINT_ERR("no init.\n");
        return NULL;
    }

    return flash->vblock[block_num];
}

vblock *vflahs_get_block_from_addr(vflash *flash, int addr)
{
    if (!flash) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return NULL;
    } else if (addr > flash->size(flash)) {
        PRINT_ERR("%s() invalid addr.\n", __func__);
        return NULL;
    }

    int block = flash->calc_block_from_addr(flash, addr);

    return flash->get_block(flash, block);
}
int vflahs_calc_block_from_addr(vflash *flash, int addr)
{
    if (!flash) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (addr > flash->size(flash)) {
        PRINT_ERR("%s() invliad addr.\n", __func__);
        return -1;
    }

    return addr / flash->block_size(flash);
}
int vflahs_calc_page_from_addr(vflash *flash, int addr)
{
    if (!flash) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (addr > flash->size(flash)) {
        PRINT_ERR("%s() invliad addr.\n", __func__);
        return -1;
    }

    addr %= flash->block_size(flash);

    return addr / flash->page_size(flash);
}
int vflahs_calc_unit_from_addr(vflash *flash, int addr)
{
    if (!flash) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (addr > flash->size(flash)) {
        PRINT_ERR("%s() invliad addr.\n", __func__);
        return -1;
    }

    addr %= flash->block_size(flash);
    addr %= flash->page_size(flash);

    return addr / sizeof(VUNIT_TYPE);
}
int vflash_calc_block_addr(vflash *flash, int blk)
{
    if (!flash || (blk >= flash->vblock_amt)) {
        PRINT_ERR("invalid args , flash(%p) || get block(%d)\n", flash, blk);
        return -1;
    } else if (!flash->vblock) {
        PRINT_ERR("no init.\n");
        return -1;
    }

    return blk * flash->block_size(flash);
}
int vflash_rdump(vflash *flash, int unit_per_line, int addr, int size)
{
    if (!flash || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }

    range_line_bar(unit_per_line);
    printf(" flash rdump %08d - %08d ( total xxxx bytes )\n", addr, addr + size);
    range_line_bar(unit_per_line);
    // -------------------------------------------------- ... 
    // | flash rdump 00000000 - 00000000 ( total xxxx bytes )
    // ---------------------------------------------
    // | 00000000 - 00000000 | FFFFFFFF FFFFFFFF ... FFFFFFFF |

    int blk = flash->calc_block_from_addr(flash, addr);
    int pag = flash->calc_page_from_addr(flash, addr);
    int uni = flash->calc_unit_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    vpage *page = block->get_page(block, pag);
    int unit_amt = size / sizeof(VUNIT_TYPE);
    if (size % sizeof(VUNIT_TYPE)) { unit_amt++; }

    int uidx = 0;
    do {

        // | 00000000 - 00000000 |  
        if (!(uidx % unit_per_line)) {
            if (uidx) { printf("|\n"); }
            printf("| %08d - %08d | ", addr + (uidx * 4), addr + ((uidx + unit_per_line) * 4));
        }

        //                         xxxxxxxx xxxxxxxx xxxxxxxx ... 
        VUNIT_TYPE data = page->vunit[uni].data;
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

        uni++;

        if ((uni) >= page->vunit_amt) {

            uni = 0;
            page = page->next;
            if (!page) {
                block = block->next;
                if (!block) {
                    if ((uidx + 1) != unit_amt) {
                        PRINT_ERR("%s() not expect error !!\n", __func__);
                        return -1;
                    }
                } else {
                    page = block->get_page(block, 0);
                    if (!page) {
                        PRINT_ERR("%s() block get first page failed !!\n", __func__);
                        return -1;
                    }
                }
            }
        }

    } while (++uidx < unit_amt);

    if (uidx % unit_per_line) {
        int left = (unit_per_line - (uidx % unit_per_line));
        for (int fill = 0; fill < left; ++fill) {
            printf("         ");
        }
    }
    printf("|\n");

    range_line_bar(unit_per_line);

    return 0;
}
int vflash_dump(vflash *flash, int unit_per_line, int addr, int size)
{
    if (!flash || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }

    // -------------------------------------------------- ... 
    // | flash dump 00000000 - 00000000 ( total xxxx bytes )
    // ---------------------------------------------
    // | 00000000 - 00000000 | FFFFFFFF FFFFFFFF ... FFFFFFFF |

    range_line_bar(unit_per_line);
    printf(" flash dump %08d - %08d ( total xxxx bytes )\n", addr, addr + size);
    range_line_bar(unit_per_line);

    int blk = flash->calc_block_from_addr(flash, addr);
    int pag = flash->calc_page_from_addr(flash, addr);
    int uni = flash->calc_unit_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    vpage *page = block->get_page(block, pag);
    int unit_amt = size / sizeof(VUNIT_TYPE);
    if (size % sizeof(VUNIT_TYPE)) { unit_amt++; }

    int uidx = 0;
    do {

        // | 00000000 - 00000000 |  
        if (!(uidx % unit_per_line)) {
            if (uidx) { printf("|\n"); }
            printf("| %08d - %08d | ", addr + (uidx * 4), addr + ((uidx + unit_per_line) * 4));
        }

        //                         xxxxxxxx xxxxxxxx xxxxxxxx ... 
        printf("%08X ", page->vunit[uni].data);

        uni++;

        if ((uni) >= page->vunit_amt) {

            uni = 0;
            page = page->next;
            if (!page) {
                block = block->next;
                if (!block) {
                    if ((uidx + 1) != unit_amt) {
                        PRINT_ERR("%s() not expect error !!\n", __func__);
                        return -1;
                    }
                } else {
                    page = block->get_page(block, 0);
                    if (!page) {
                        PRINT_ERR("%s() block get first page failed !!\n", __func__);
                        return -1;
                    }
                }
            }
        }

    } while (++uidx < unit_amt);

    if (uidx % unit_per_line) {
        int left = (unit_per_line - (uidx % unit_per_line));
        for (int fill = 0; fill < left; ++fill) {
            printf("         ");
        }
    }
    printf("|\n");

    range_line_bar(unit_per_line);

    return 0;
}
int vflash_info(vflash *flash, int unit_per_line, int align, int addr, int size)
{
    if (!flash || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }


    range_info_line_bar(unit_per_line, align);
    printf(" flash info %08d - %08d ( total xxxx bytes )\n", addr, addr + size);
    range_info_line_bar(unit_per_line, align);
      
    // | 00000000 - 00000000 | [■ (??,??)] [■ (???,???)] [■ (xx,oo)] |

    char ufmt[32] = {0};
    snprintf(ufmt, 32, "[■ (%%%dd,%%%dd)] ", align, align);

    char nfmt[32] = {0};
    snprintf(nfmt, 32, "[□ (%%%dd,%%%dd)] ", align, align);

    int blk = flash->calc_block_from_addr(flash, addr);
    int pag = flash->calc_page_from_addr(flash, addr);
    int uni = flash->calc_unit_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    vpage *page = block->get_page(block, pag);
    int unit_amt = size / sizeof(VUNIT_TYPE);
    if (size % sizeof(VUNIT_TYPE)) { unit_amt++; }

    int uidx = 0;
    do {

        // | 00000000 - 00000000 |  
        if (!(uidx % unit_per_line)) {
            if (uidx) { printf("|\n"); }
            printf("| %08d - %08d | ", addr + (uidx * 4), addr + ((uidx + unit_per_line) * 4));
        }

        //                         xxxxxxxx xxxxxxxx xxxxxxxx ... 
        if (page->vunit[uni].used) {
            printf(ufmt, page->vunit[uni].erase, page->vunit[uni].write);
        } else {
            printf(nfmt, page->vunit[uni].erase, page->vunit[uni].write);
        }
        uni++;

        if ((uni) >= page->vunit_amt) {

            uni = 0;
            page = page->next;
            if (!page) {
                block = block->next;
                if (!block) {
                    if ((uidx + 1) != unit_amt) {
                        PRINT_ERR("%s() not expect error !!\n", __func__);
                        return -1;
                    }
                } else {
                    page = block->get_page(block, 0);
                    if (!page) {
                        PRINT_ERR("%s() block get first page failed !!\n", __func__);
                        return -1;
                    }
                }
            }
        }

    } while (++uidx < unit_amt);

    if (uidx % unit_per_line) {
        int left = (unit_per_line - (uidx % unit_per_line));
        for (int fill = 0; fill < left; ++fill) {
            printf("    ");
            for (int an = 0; an < (align * 2 + 1); ++an) printf(" ");
            printf("   ");
        }
    }
    printf("|\n");

    range_info_line_bar(unit_per_line, align);

    return 0;
}

int vflash_read(vflash *flash, int addr, int size, unsigned char *buff)
{
    if (!flash || !buff || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }

    // |<- block ->|<- block ->|<- block ->|<- block ->|
    //      ■■■■■■■ ■■■■■■■■■■■ ■■■■■■■■■■■ ■■■■■
    //     | first |        middle         | end |

    int blk = flash->calc_block_from_addr(flash, addr);
    int pag = flash->calc_page_from_addr(flash, addr);
    int uni = flash->calc_unit_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    if (!block) {
        PRINT_ERR("%s() flash get block failed.\n", __func__);
        return -1;
    }
    vpage *page = block->get_page(block, pag);
    if (!page) {
        PRINT_ERR("%s() block get page failed.\n", __func__);
        return -1;
    }

    while(size > 0) {

        VUNIT_TYPE tmp = page->get_unit(page, uni);
        memcpy(buff, &tmp, sizeof(VUNIT_TYPE));

        if (++uni == page->vunit_amt) {
            uni = 0;
            page = page->next;
            if (!page) {
                block = block->next;
                if (!block) {
                    PRINT_ERR("%s() wrong size ? .\n", __func__);
                    return -1;
                }
                page = block->get_page(block, 0);
            }
        }
        size -= sizeof(VUNIT_TYPE);
        buff += sizeof(VUNIT_TYPE);

    }

    return 0;
}
int vflash_write(vflash *flash, unsigned int addr, unsigned int size, const unsigned char *buff)
{
    if (!flash || !buff || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }

    // |<- block ->|<- block ->|<- block ->|<- block ->|
    //      ■■■■■■■ ■■■■■■■■■■■ ■■■■■■■■■■■ ■■■■■
    //     | first |        middle         | end |

    int blk = flash->calc_block_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    if (!block) {
        PRINT_ERR("%s() flash get block from addr %d failed.\n", __func__, addr);
        return -1;
    }

    int off = addr - block->addr(block);
    int rsz = size;
    if ((addr + rsz) > (block->addr(block) + block->size(block))) {
        rsz = block->size(block) - off;
    }

    if (block->write(block, off, buff, rsz)) {
        PRINT_ERR("%s() block write off %d , size %d failed.\n", __func__, off, rsz);
        return -1;
    } else {
        size -= rsz;
        buff += rsz;
    }

    // front part only
    if (!size) {
        return 0;
    }

    // middle part
    rsz = block->size(block);
    while(size > rsz) {
        block = flash->get_block(flash, ++blk);
        if (block->write(block, 0, buff, rsz)) {
            PRINT_ERR("%s() block %d write off 0 size %d failed.\n", __func__, blk, rsz);
            return -1;
        }
        size -= rsz;
        buff += rsz;
    }

    if (!size) {
        return 0;
    }

    // last part
    block = flash->get_block(flash, ++blk);
    if (block->write(block, 0, buff, size)) {
        PRINT_ERR("%s() block %d write off 0 size %d failed.\n", __func__, blk, size);
        return -1;
    }

    return 0;
}
int vflash_erase(vflash *flash, int addr, int size)
{
    if (!flash || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (addr % flash->block_size(flash)) {
        PRINT_ERR("%s() addr not align (block size %d).\n", __func__, flash->block_size(flash));
        return -1;
    } else if (size % flash->block_size(flash)) {
        PRINT_ERR("%s() size not align (block size %d).\n", __func__, flash->block_size(flash));
        return -1;
    }

    int count = 0;

    while(count != size) {
        vblock *block = flash->get_block_from_addr(flash, addr);
        block->erase(block);
        addr += flash->block_size(flash);
        count += flash->block_size(flash);
    }

    return 0;
}

int vflash_config_update(vflash *flash, int addr, int size)
{
    if (!flash || !size) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((addr + size) > flash->size(flash)) {
        PRINT_ERR("%s() out of range.\n", __func__);
        return -1;
    }

    // |<- block ->|<- block ->|<- block ->|<- block ->|
    //      ■■■■■■■ ■■■■■■■■■■■ ■■■■■■■■■■■ ■■■■■
    //     | first |        middle         | end |

    int blk = flash->calc_block_from_addr(flash, addr);
    vblock *block = flash->get_block(flash, blk);
    if (!block) {
        PRINT_ERR("%s() flash get block from addr %d failed.\n", __func__, addr);
        return -1;
    }

    int off = addr - block->addr(block);
    int rsz = size;
    if ((addr + rsz) > (block->addr(block) + block->size(block))) {
        rsz = block->size(block) - off;
    }

    if (flash->fblock_update(flash, blk, off, rsz)) {
        PRINT_ERR("%s() flash update blk %d off %d , size %d failed.\n", __func__, blk, off, rsz);
        return -1;
    } else {
        size -= rsz;
    }

    // front part only
    if (!size) {
        return 0;
    }

    // middle part
    rsz = block->size(block);
    while(size > rsz) {
        if (flash->fblock_update(flash, ++blk, 0, rsz)) {
            PRINT_ERR("%s() flash update blk %d off 0 , size %d failed.\n", __func__, blk, rsz);
            return -1;
        }
        size -= rsz;
    }

    if (!size) {
        return 0;
    }

    // last part
    if (flash->fblock_update(flash, ++blk, 0, size)) {
        PRINT_ERR("%s() flash update blk %d off 0 , size %d failed.\n", __func__, blk, size);
        return -1;
    }

    return 0;
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
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    if (access(file, F_OK)) {
        PRINT_ERR("file not exist.\n");
        return -1;
    }

    // open config file
    FILE *fd = fopen("flash.instance", "r");
    int load_result = 0;
    if (!fd) {
        PRINT_ERR("fopen failed : %s\n", strerror(errno));
        return -1;
    } else if (fseek(fd, 0, SEEK_END)) {
        PRINT_ERR("fseek to end failed : %s\n", strerror(errno));
        load_result = -1;
    } else if (ftell(fd) != flash->size(flash)) {
        PRINT_ERR("file size mismatch.\n");
        load_result = -1;
    } else if (fseek(fd, 0, SEEK_SET)) {
        PRINT_ERR("fseek to start failed : %s\n", strerror(errno));
        load_result = -1;
    } else {

        int block_size = flash->block_size(flash);
        unsigned char *rdata = (unsigned char *)malloc(block_size);

        for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
            memset(rdata, 0, block_size);
            vblock *block = flash->vblock[bidx];

            if (fread(rdata, block_size, 1, fd) < 0) {
                PRINT_ERR("fread failed : %s.\n", strerror(errno));
                load_result = -1;
                break;
            } else if (block->write(block, 0, rdata, block_size)) {
                PRINT_ERR("block %d read failed.\n", bidx);
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
        PRINT_ERR("invaild args.\n");
        return -1;
    }

    flash->used = 0;
    flash->vblock_amt = vblock_amt;
    flash->vpage_amt = vpage_amt;
    flash->vunit_amt = vunit_amt;

    flash->vblock = (vblock **)malloc(sizeof(vblock *) * flash->vblock_amt);
    if (!flash->vblock) {
        PRINT_ERR("vblock point array allocate failed.\n");
        return -1;
    }
    memset(flash->vblock, 0, sizeof(vblock *) * flash->vblock_amt);

    for (int idx = 0; idx < flash->vblock_amt; ++idx) {
        vblock *block = vblock_new();
        if (!block) {
            PRINT_ERR("vblock %d new failed.\n", idx);
            return -1;
        } else if (block->init(block, flash->vpage_amt, flash->vunit_amt)) {
            PRINT_ERR("vblock %d init failed.\n", idx);
            block->destroy(block);
            return -1;
        }

        block->seq = idx;
        flash->vblock[idx] = block;
        if (idx) {
            flash->vblock[idx - 1]->next = block;
        }
    }

    return 0;
}

vflash *vflash_dupc(vflash *flash)
{
    if (!flash) {
        PRINT_ERR("invliad args.\n");
        return NULL;
    }

    vflash *flash_new = vflash_new();
    if (!flash_new) {
        PRINT_ERR("flash new failed\n");
    }

    flash_new->used       = flash->used;
    flash_new->vblock_amt = flash->vblock_amt;
    flash_new->vpage_amt  = flash->vpage_amt;
    flash_new->vunit_amt  = flash->vunit_amt;

    flash_new->vblock = (vblock **)malloc(sizeof(vblock *) * flash_new->vblock_amt);
    if (!flash_new->vblock) {
        PRINT_ERR("vblock point array allocate failed.\n");
        flash_new->destroy(flash_new);
        return NULL;
    }
    memset(flash_new->vblock, 0, sizeof(vblock *) * flash_new->vblock_amt);

    for (int idx = 0; idx < flash_new->vblock_amt; ++idx) {
        flash_new->vblock[idx] = flash->vblock[idx]->dupc(flash->vblock[idx]);
        if (!flash_new->vblock[idx]) {
            PRINT_ERR("block %d dupc failed\n", idx);
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
        PRINT_ERR("allocate failed\n");
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
    flash->get_block_from_addr = vflahs_get_block_from_addr;
    flash->calc_block_addr = vflash_calc_block_addr;
    flash->calc_block_from_addr = vflahs_calc_block_from_addr;
    flash->calc_page_from_addr = vflahs_calc_page_from_addr;
    flash->calc_unit_from_addr = vflahs_calc_unit_from_addr;

    // new feature 2022 08 23
    flash->fdump = vflash_file_dump;
    flash->fload = vflash_file_load;

    // new feature 2022 08 30
    flash->fimport = vflash_config_import;
    flash->fexport = vflash_config_export;

    flash->fupdate = vflash_config_update;
    flash->fblock_update = vflash_config_block_update;
    flash->fpage_update = vflash_config_page_update;

    flash->check_addr = vflash_check_addr;
    flash->check_val = vflash_check_val;

    flash->read = vflash_read;
    flash->write = vflash_write;
    flash->erase = vflash_erase;

    flash->dump = vflash_dump;
    flash->rdump = vflash_rdump;
    flash->info = vflash_info;

    return flash;
}
