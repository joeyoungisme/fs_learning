#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "opt.h"
#include "timer.h"
#include "misc.h"
#include "myfs.h"

#include "ff.h"

#define TAB_STR             "    "

int myfs_lfs_init(myfs **lfs, int blk, int pag, int unit)
{
    PRINT("!! LFS INIT !!\n");

    *lfs = myfs_new();
    if (!(*lfs)) {
        PRINT_ERR("%s() (*lfs) new failed\n", __func__);
        return -1;
    }
    if ((*lfs)->init((*lfs), MYFS_LITTLEFS, blk, pag, unit)) {
        PRINT_ERR("%s() (*lfs) init failed\n", __func__);
        return -1;
    }
    if ((*lfs)->mount((*lfs))) {
        PRINT_ERR("%s() (*lfs) mount failed\n", __func__);

        if ((*lfs)->format((*lfs))) {
            PRINT_ERR("%s() (*lfs) format failed\n", __func__);
            return -1;
        } else if ((*lfs)->mount((*lfs))) {
            PRINT_ERR("%s() lfs mount failed (again).\n", __func__);
            return -1;
        }
    }

    return 0;
}

void lfs_uninit(myfs **lfs)
{
    PRINT("!! LFS UNINIT !!\n");

    if ((*lfs)->unmount((*lfs))) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    (*lfs)->destroy((*lfs));
}

int myfs_fat_init(myfs **fatfs, int blk, int pag, int unit)
{
    PRINT("!! FAT INIT !!\n");

    uint32_t freeClust = 0;
    uint32_t totalBlocks = 0;
    uint32_t freeBlocks = 0;

    *fatfs = myfs_new();
    if (!(*fatfs)) {
        PRINT_ERR("%s() (*fatfs) new failed\n", __func__);
        return -1;
    } else if ((*fatfs)->init((*fatfs), MYFS_FATFS, blk, pag, unit)) {
        PRINT_ERR("%s() (*fatfs) init failed\n", __func__);
        return -1;
    } else if ((*fatfs)->mount((*fatfs))) {
        PRINT_ERR("%s() (*fatfs) mount failed\n", __func__);
        return -1;
    } else if ((*fatfs)->get_free_clust((*fatfs), &freeClust)) {

        PRINT_ERR("%s() gree free clust failed.\n", __func__);

        if ((*fatfs)->format((*fatfs))) {
            PRINT_ERR("%s() (*fatfs) format failed\n", __func__);
            return -1;
        } else if ((*fatfs)->get_free_clust((*fatfs), &freeClust)) {
            PRINT_ERR("%s() gree free clust failed (again).\n", __func__);
            return -1;
        }
    }

    totalBlocks = ((*fatfs)->fat->head->core.n_fatent - 2) * (*fatfs)->fat->head->core.csize;
    freeBlocks = freeClust * (*fatfs)->fat->head->core.csize;

    PRINT_RAW("Free Cluster: %u \n", freeClust);
    PRINT_RAW("number of clusters: %ld \n", (*fatfs)->fat->head->core.n_fatent - 2);
    PRINT_RAW("sector per cluster: %u \n", (*fatfs)->fat->head->core.csize);
    PRINT_RAW("Total blocks: %u (%u Mb)\n", totalBlocks, totalBlocks / 2000);
    PRINT_RAW("Free blocks: %u (%u Mb)\n", freeBlocks, freeBlocks / 2000);

    return 0;
}
void fat_uninit(myfs **fat)
{
    PRINT("!! FAT UNINIT !!\n");

    if ((*fat)->unmount((*fat))) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    (*fat)->destroy((*fat));

    return;
}

static MYFS_TYPE type = MYFS_TYPE_AMOUNT;
static int open_flag = 0;
// --dir
static unsigned char mkdir_flag = 0;
// --rm
static unsigned char remove_flag = 0;
// --cyc
static unsigned char cycle_flag = 0;
// --w
static unsigned char write_flag = 0;
// --get
static unsigned char get_file_flag = 0;
static char get_file_path[512] = {0};
// --put
static unsigned char put_file_flag = 0;
static char put_file_path[512] = {0};
// --chk
static unsigned char check_flag = 0;
// --wcnt [count]
static unsigned int write_count = 1;
// --cycc [count]
static unsigned int cycle_count = 0;
// --sync [times]
static unsigned int times_of_sync = 0;
// --inf
static unsigned char infinity_flag = 0;
// --r
static unsigned char read_flag = 0;
// --list
static unsigned char list_flag = 0;
static char write_content[512] = {0};
// --dump
static unsigned char dump_flag = 0;
// --dblk
static int dblk_flag = -1;
// --dpage
static int dpag_flag = -1;
// --dnl
static int dnl_flag = 8;
// --dal
static int dal_flag = 2;
static char file_name[128] = {0};
static char dir_name[128] = {0};
static char list_path[128] = {0};
static myfs_dir dir;
static myfs_file file;
static myfs_file_info info;

int opt_type(char *args)
{
    PRINT("You Choise File System Type is : %s\n", args);

    if (!strncmp(args, "lfs", strlen(args))) {
        type = MYFS_LITTLEFS;
        open_flag = LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND;
    } else if (!strncmp(args, "fat", strlen(args))) {
        type = MYFS_FATFS;
        open_flag = FA_OPEN_APPEND | FA_WRITE;
    }
    return 0;
}
int opt_dump(char *args)
{
    if (!args) { return 0; }

    if (!strncmp(args, "raw", strlen(args))) {
        dump_flag |= 0x01;
    } else if (!strncmp(args, "rraw", strlen(args))) {
        dump_flag |= 0x02;
    } else if (!strncmp(args, "info", strlen(args))) {
        dump_flag |= 0x04;
    }

    return 0;
}
int opt_dal(char *args)
{
    if (!strlen(args)) { return -1; }

    dal_flag = atoi(args);

    return 0;
}
int opt_dnl(char *args)
{
    if (!strlen(args)) { return -1; }

    dnl_flag = atoi(args);

    return 0;
}
int opt_dblk(char *args)
{
    dblk_flag = atoi(args);

    return 0;
}
int opt_dpag(char *args)
{
    dpag_flag = atoi(args);

    return 0;
}
int opt_list(char *args)
{
    list_flag = 1;
    memset(list_path, 0, sizeof(list_path));
    snprintf(list_path, sizeof(list_path), "%s", args);

    return 0;
}
int opt_file(char *args)
{
    PRINT("%s\n", args);
    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), "%s", args);
    return 0;
}
int opt_read(char *args)
{
    read_flag = 1;

    return 0;
}
int opt_infinity(char *args)
{
    infinity_flag = 1;

    return 0;
}
int opt_check(char *args)
{
    check_flag = 1;

    return 0;
}
int opt_remove(char *args)
{
    remove_flag = 1;

    return 0;
}
int opt_cycle_count(char *args)
{
    cycle_count = atoi(args);

    return 0;
}
int opt_sync(char *args)
{
    times_of_sync = atoi(args);

    return 0;
}
int opt_wcount(char *args)
{
    write_count = atoi(args);

    return 0;
}
int opt_cycle(char *args)
{
    PRINT("%s\n", args);
    cycle_flag = 1;
    memset(write_content, 0, sizeof(write_content));
    snprintf(write_content, sizeof(write_content), "%s\n", args);

    return 0;
}
int opt_putfile(char *args)
{
    put_file_flag = 1;
    memset(put_file_path, 0, sizeof(put_file_path));
    snprintf(put_file_path, sizeof(put_file_path), "%s", args);
    return 0;
}
int opt_getfile(char *args)
{
    get_file_flag = 1;
    memset(get_file_path, 0, sizeof(get_file_path));
    snprintf(get_file_path, sizeof(get_file_path), "%s", args);
    return 0;
}

int opt_write(char *args)
{
    PRINT("%s\n", args);
    write_flag = 1;
    memset(write_content, 0, sizeof(write_content));
    snprintf(write_content, sizeof(write_content), "%s\n", args);

    return 0;
}
int opt_mkdir(char *args)
{
    mkdir_flag = 1;
    memset(dir_name, 0, sizeof(dir_name));
    snprintf(dir_name, sizeof(dir_name), "%s", args);

    return 0;
}

void opt_init(void)
{
    struct option opt = {0};

    opt.name = "type"; opt.has_arg = required_argument;
    opt_reg(opt, opt_type, "file system type , \"lfs\" or \"fat\"");

    opt.name = "dump"; opt.has_arg = required_argument;
    opt_reg(opt, opt_dump, "dump all flash info , \"raw\" or \"rraw\" of \"info\"");

    opt.name = "dnl"; opt.has_arg = required_argument;
    opt_reg(opt, opt_dnl, "dump how many item next line , please add --dump [type]");

    opt.name = "dal"; opt.has_arg = required_argument;
    opt_reg(opt, opt_dal, "dump info align , please add --dump info");

    opt.name = "dblk"; opt.has_arg = required_argument;
    opt_reg(opt, opt_dblk, "dump specific block , please add --dump [type]");

    opt.name = "dpag"; opt.has_arg = required_argument;
    opt_reg(opt, opt_dpag, "dump specific page , please add --dump [type]");

    opt.name = "dir"; opt.has_arg = required_argument;
    opt_reg(opt, opt_mkdir, "make dir , --dir [dir name]");

    opt.name = "list"; opt.has_arg = required_argument;
    opt_reg(opt, opt_list, "show list , --list [path]");

    opt.name = "cyc"; opt.has_arg = required_argument;
    opt_reg(opt, opt_cycle, "cycle write file content, please add --file [file name]");

    opt.name = "cycc"; opt.has_arg = required_argument;
    opt_reg(opt, opt_cycle_count, "set cycle count (rm -> write -> rm), please add --file [file name] --w [data]");

    opt.name = "sync"; opt.has_arg = required_argument;
    opt_reg(opt, opt_sync, "set sync to file times , please add --file [file name] --w [data]");

    opt.name = "w"; opt.has_arg = required_argument;
    opt_reg(opt, opt_write, "write file , please add --file [file name]");

    opt.name = "wcnt"; opt.has_arg = required_argument;
    opt_reg(opt, opt_wcount, "write Data Times , please add --file [file name] --w [data]");

    opt.name = "file"; opt.has_arg = required_argument;
    opt_reg(opt, opt_file, "open file by file name");

    opt.name = "put"; opt.has_arg = required_argument;
    opt_reg(opt, opt_putfile, "put file from pc to fs , please add --file [file name]");

    opt.name = "get"; opt.has_arg = required_argument;
    opt_reg(opt, opt_getfile, "get file from fs to pc , please add --file [file name]");

    opt.name = "rm"; opt.has_arg = no_argument;
    opt_reg(opt, opt_remove, "remove file , please add --file [file name]");

    opt.name = "r"; opt.has_arg = no_argument;
    opt_reg(opt, opt_read, "read file , please add --file [file name]");

    opt.name = "inf"; opt.has_arg = no_argument;
    opt_reg(opt, opt_infinity, "infinity flag , please add --file [file name] and --w [data]");

    opt.name = "chk"; opt.has_arg = no_argument;
    opt_reg(opt, opt_check, "flash check flag , address / value check");

    return;
}

int test_dump_info(myfs *fs)
{
    // info ( .info )
    vflash *flash = fs->get_flash(fs);

    flash->info(flash, dnl_flag, dal_flag, 0, flash->size(flash));

    // raw ( .dump )
//  if (dblk_flag < 0) {

//      for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
//          vblock *block = flash->get_block(flash, bidx);

//          if (dpag_flag < 0) {
//              PRINT("Dump Flash Info : Block %d.\n", bidx);
//              block->info(block, dnl_flag, dal_flag);
//          } else {
//              PRINT("Dump Flash Info : Block %d, Page %d.\n", bidx, dpag_flag);
//              vpage *page = block->get_page(block, dpag_flag);
//              page->info(page, dnl_flag, dal_flag);
//          }
//      }
//  } else {
//      vblock *block = flash->get_block(flash, dblk_flag);
//      if (dpag_flag < 0) {
//          PRINT("Dump Flash Info : Block %d.\n", dblk_flag);
//          block->info(block, dnl_flag, dal_flag);
//      } else {
//          PRINT("Dump Flash Info : Block %d, Page %d.\n", dblk_flag, dpag_flag);
//          vpage *page = block->get_page(block, dpag_flag);
//          page->info(page, dnl_flag, dal_flag);
//      }
//  }

    return 0;
}
int test_dump_read(myfs *fs)
{
    // rraw ( .rdump )
    vflash *flash = fs->get_flash(fs);

    flash->rdump(flash, dnl_flag, 0, flash->size(flash));

    // raw ( .dump )
//  if (dblk_flag < 0) {

//      for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
//          vblock *block = flash->get_block(flash, bidx);

//          if (dpag_flag < 0) {
//              PRINT("Dump Flash Info : Block %d.\n", bidx);
//              block->rdump(block, dnl_flag);
//          } else {
//              PRINT("Dump Flash Info : Block %d, Page %d.\n", bidx, dpag_flag);
//              vpage *page = block->get_page(block, dpag_flag);
//              page->rdump(page, dnl_flag);
//          }
//      }
//  } else {
//      vblock *block = flash->get_block(flash, dblk_flag);
//      if (dpag_flag < 0) {
//          PRINT("Dump Flash Info : Block %d.\n", dblk_flag);
//          block->rdump(block, dnl_flag);
//      } else {
//          PRINT("Dump Flash Info : Block %d, Page %d.\n", dblk_flag, dpag_flag);
//          vpage *page = block->get_page(block, dpag_flag);
//          page->rdump(page, dnl_flag);
//      }
//  }

    return 0;
}
int test_dump_raw(myfs *fs)
{
    vflash *flash = fs->get_flash(fs);

    flash->dump(flash, dnl_flag, 0, flash->size(flash));

//  // raw ( .dump )
//  if (dblk_flag < 0) {

//      for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {
//          vblock *block = flash->get_block(flash, bidx);

//          if (dpag_flag < 0) {
//              PRINT("Dump Flash Info : Block %d.\n", bidx);
//              block->dump(block, dnl_flag);
//          } else {
//              PRINT("Dump Flash Info : Block %d, Page %d.\n", bidx, dpag_flag);
//              vpage *page = block->get_page(block, dpag_flag);
//              page->dump(page, dnl_flag);
//          }
//      }
//  } else {
//      vblock *block = flash->get_block(flash, dblk_flag);
//      if (dpag_flag < 0) {
//          PRINT("Dump Flash Info : Block %d.\n", dblk_flag);
//          block->dump(block, dnl_flag);
//      } else {
//          PRINT("Dump Flash Info : Block %d, Page %d.\n", dblk_flag, dpag_flag);
//          vpage *page = block->get_page(block, dpag_flag);
//          page->dump(page, dnl_flag);
//      }
//  }

    return 0;
}
int test_open(myfs *fs)
{
    Timer otimer;

    PRINT(" ---------------- [ %6s ] ----------------\n", "OPEN");
    PRINT("OPEN : %s\n", file_name);
    tm_set_ms(&otimer, 0);
    if (fs->open(fs, &file, file_name, open_flag)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return -1;
    }

    return tm_stopwatch(otimer);
}
int test_close(myfs *fs)
{
    Timer ctimer;

    PRINT(" ---------------- [ %6s ] ----------------\n", "CLOSE");
    tm_set_ms(&ctimer, 0);
    if (fs->close(fs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return -1;
    }
    return tm_stopwatch(ctimer);
}
int test_remove(myfs *fs)
{
    Timer rtimer;

    PRINT(" ---------------- [ %6s ] ----------------\n", "REMOVE");
    tm_set_ms(&rtimer, 0);
    if (fs->remove(fs, file_name)) {
        PRINT_ERR("%s() remove failed.\n", __func__);
        return -1;
    }
    return tm_stopwatch(rtimer);
}
int test_write(myfs *fs)
{
    Timer wtimer;
    Timer stimer;

    PRINT(" ---------------- [ %6s ] ----------------\n", "WRITE");
    unsigned int w_total = 0;
    int count = write_count;
    int wlen = strlen(write_content);

    tm_set_ms(&wtimer, 0);
    if (infinity_flag || count) {
        if (wlen) {
            do {
                PRINT("WRITE : %s", write_content);
                int wres = fs->write(fs, &file, write_content, wlen);
                if (wres == 0) { break; }
                else if (wres < 0) {
                    PRINT_ERR("%s() fs write failed.\n", __func__);
                    return -1;
                }
                w_total += wres;

                if (times_of_sync && !(count % times_of_sync)) {
                    PRINT(" ---------------- [ %6s ] ----------------\n", "SYNC");
                    tm_set_ms(&stimer, 0);
                    fs->sync(fs, &file);
                    PRINT("SYNC : every %d times.\n", times_of_sync);
                    PRINT_RAW("SYNC   Spent %d ms.\n", tm_stopwatch(stimer));
                }
            } while(infinity_flag || --count);
        }
    } else {
        int wres = fs->write(fs, &file, write_content, wlen);
        if (wres < 0) {
            PRINT_ERR("%s() write failed.\n", __func__);
        } else {
            w_total += wres;
        }
    }
    PRINT("Total Write %d bytes.\n", w_total);
    return tm_stopwatch(wtimer);
}

int test_copy_to_fs(myfs *fs, FILE *fd)
{
    if (!fs || !fd) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    Timer ptimer;
    // Timer stimer;

    unsigned char copy_buff[2048] = {0};

    tm_set_ms(&ptimer, 0);
    while(1)
    {
        int rlen = fread(copy_buff, sizeof(unsigned char), 2048, fd);
        if (rlen < 0) {
            PRINT_ERR("%s() fread failed : %s.\n", __func__, strerror(errno));
            return -1;
        } else if (!rlen) {
            break;
        }

        int wlen = fs->write(fs, &file, copy_buff, rlen);
        if (wlen < 0) {
            PRINT_ERR("%s() fs write failed.\n", __func__);
            return -1;
        }

        if (rlen != wlen) {
            PRINT_ERR("%s() read write size mismatch r %d byte , w %d byte.\n", __func__, rlen, wlen);
            return -1;
        }
    }

    return tm_stopwatch(ptimer);
}
int test_copy_from_fs(myfs *fs, FILE *fd)
{
    if (!fs || !fd) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    Timer gtimer;
    // Timer stimer;

    unsigned char copy_buff[2048] = {0};

    tm_set_ms(&gtimer, 0);
    while(1)
    {
        int rlen = fs->read(fs, &file, copy_buff, sizeof(copy_buff));
        if (rlen < 0) {
            PRINT_ERR("%s() read failed.\n", __func__);
            break;
        } else if (!rlen) {
            break;
        }
        int wlen = fwrite(copy_buff, sizeof(unsigned char), rlen, fd);
        if (wlen < 0) {
            PRINT_ERR("%s() fwrite failed : %s\n", __func__, strerror(errno));
        }
        
        if (rlen != wlen) {
            PRINT_ERR("%s() read write size mismatch r %d byte , w %d byte.\n", __func__, rlen, wlen);
            return -1;
        }
    }

    return tm_stopwatch(gtimer);
}
int test_get_file(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    FILE *output = fopen(get_file_path, "wb");
    if (!output) {
        PRINT_ERR("%s() fopen failed : %s\n", __func__, strerror(errno));
        return -1;
    }

    myfs_file file;

    if (fs->open(fs, &file, file_name, LFS_O_RDWR)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return -1;
    }

    for(;;)
    {
        char readBuff[512];
        int rlen = fs->read(fs, &file, readBuff, sizeof(readBuff)-1);
        if (rlen < 0) {
            PRINT_ERR("%s() read failed.\n", __func__);
            break;
        } else if (!rlen) {
            break;
        }
        if (fwrite(readBuff, rlen, 1, output) != 1) {
            PRINT_ERR("%s() fwrite failed : %s\n", __func__, strerror(errno));
        }
    }

    if (fs->close(fs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return -1;
    }

    fclose(output);

    return 0;
}

int test_show_file(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    myfs_file file;

    if (fs->open(fs, &file, file_name, LFS_O_RDWR | LFS_O_CREAT)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return -1;
    }

    PRINT_RAW("```\n");
    for(;;)
    {
        char readBuff[512];
        int rlen = fs->read(fs, &file, readBuff, sizeof(readBuff)-1);
        if (rlen < 0) {
            // PRINT_ERR("%s() read failed.\n", __func__);
            return -1;
        } else if (!rlen) {
            break;
        }
        readBuff[rlen] = '\0';
        PRINT_RAW("%s", readBuff);
    }
    PRINT_RAW("\n```\n");

    if (fs->close(fs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return -1;
    }

    return 0;
}

int test_list_dir(myfs *fs, char *path, int tab);
int test_list_file(myfs *fs, char *path, int tab);
int test_list_all(myfs *fs, char *path, int tab);

int test_list_dir(myfs *fs, char *path, int tab)
{
    if (!fs || !path) {
        return -1;
    }

    myfs_dir dir;
    myfs_file_info info;

    if (fs->opendir(fs, &dir, path)) {
        PRINT_ERR("%s() fs opendir failed\n", __func__);
        return -1;
    }

    int totalDirs = 0;

    for(;;) {

        int rc = fs->readdir(fs, &dir, &info);

        if (fs->type == MYFS_LITTLEFS) {
            if (rc < 0) {
                PRINT_ERR("%s() readdir failed (%d).\n", __func__, rc);
                return -1;
            } else if (!rc) {
                break;
            } else if (info.lfs.type & LFS_TYPE_DIR) {
                for (int tidx = 0; tidx < tab; ++tidx) { PRINT_RAW(TAB_STR); }
                PRINT_RAW("[LFS] [DIR]  %s\n", info.lfs.name);
                if (!strcmp(info.lfs.name, ".")) {
                } else if (!strcmp(info.lfs.name, "..")) {
                } else {
                    char sub_path[280] = {0};
                    snprintf(sub_path, 280, "%s/%s", path, info.lfs.name);
                    test_list_all(fs, sub_path, tab + 1);
                }
                totalDirs++;
            }
        } else if (fs->type == MYFS_FATFS) {
            if (rc) {
                break;
            } else if (info.fat.fname[0] == '\0') {
                break;
            } else if (info.fat.fattrib & AM_DIR) {
                for (int tidx = 0; tidx < tab; ++tidx) { PRINT_RAW(TAB_STR); }
                PRINT_RAW("[FAT] [DIR]  [%04d/%02d/%02d %02d:%02d:%02d] ",
                        (info.fat.fdate >> 9) + 1980, (info.fat.fdate >> 5) & 0x0F, (info.fat.fdate & 0x1F),
                        ((info.fat.ftime >> 11) & 0x1F) - 1, (info.fat.ftime >> 5) & 0x3F, (info.fat.ftime & 0x1F) * 2);
                PRINT_RAW("%s\n", info.fat.fname);
                char sub_path[64] = {0};
                snprintf(sub_path, 64, "%s/%s", path, info.fat.fname);
                test_list_all(fs, sub_path, tab + 1);
                totalDirs++;
            }
        } else {
            break;
        }
    }

    if (fs->closedir(fs, &dir)) {
        PRINT_ERR("%s() fs closedir failed\n", __func__);
        return -1;
    }

    return totalDirs;
}
int test_list_file(myfs *fs, char *path, int tab)
{
    if (!fs || !path) {
        return -1;
    }

    myfs_dir dir;
    myfs_file_info info;

    if (fs->opendir(fs, &dir, path)) {
        PRINT_ERR("%s() fs opendir failed\n", __func__);
        return -1;
    }

    uint32_t totalFiles = 0;

    for(;;) {

        int rc = fs->readdir(fs, &dir, &info);

        if (fs->type == MYFS_LITTLEFS) {
            if (rc < 0) {
                PRINT_ERR("%s() readdir failed (%d).\n", __func__, rc);
                return -1;
            } else if (!rc) {
                break;
            } else if (!(info.lfs.type & LFS_TYPE_DIR)) {
                for (int tidx = 0; tidx < tab; ++tidx) { PRINT_RAW(TAB_STR); }
                PRINT_RAW("[LFS] [FILE] %s (%d byte)\n", info.lfs.name, info.lfs.size);
                totalFiles++;
            }
        } else if (fs->type == MYFS_FATFS) {

            if (rc) {
                break;
            } else if (info.fat.fname[0] == '\0') {
                break;
            } else if (!(info.fat.fattrib & AM_DIR)) {
                for (int tidx = 0; tidx < tab; ++tidx) { PRINT_RAW(TAB_STR); }
                PRINT_RAW("[FAT] [FILE] [%04d/%02d/%02d %02d:%02d:%02d] ",
                        (info.fat.fdate >> 9) + 1980, (info.fat.fdate >> 5) & 0x0F, (info.fat.fdate & 0x1F),
                        ((info.fat.ftime >> 11) & 0x1F) - 1, (info.fat.ftime >> 5) & 0x3F, (info.fat.ftime & 0x1F) * 2);
                PRINT_RAW("%s (%ld byte)\n", info.fat.fname, info.fat.fsize);
                totalFiles++;
            }
        } else {
            break;
        }
    }

    if (fs->closedir(fs, &dir)) {
        PRINT_ERR("%s() fs closedir failed\n", __func__);
        return -1;
    }

    return totalFiles;
}
int test_list_all(myfs *fs, char *path, int tab)
{
    if (!fs) {
        return -1;
    }

    char root[] = "/";

    if (!path) {
        path = root;
    }

    int totalDirs = test_list_dir(fs, path, tab);
    int totalFiles = test_list_file(fs, path, tab);

    for (int tidx = 0; tidx < tab; ++tidx) { PRINT_RAW(TAB_STR); }
    PRINT_RAW("[TOTAL : %u dirs, %u files]\n", totalDirs, totalFiles);

    return 0;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    opt_init();

    if (argc < 2) {
        opt_usage(argv[0]);
        opt_free();
        exit(EXIT_FAILURE);
    } else {
        opt_run(argc, argv);
        opt_free();
    }

    myfs *fs = NULL;

    Timer itimer;
    unsigned int ispent = 0;
    tm_set_ms(&itimer, 0);
    if (type == MYFS_LITTLEFS) {
        if (myfs_lfs_init(&fs, 32, 64, 512)) {
            exit(EXIT_FAILURE);
        }
    } else if (type == MYFS_FATFS) {
        if (myfs_fat_init(&fs, 32, 64, 512)) {
            exit(EXIT_FAILURE);
        }
    } else {
        PRINT_ERR("Unknown type.\n");
        exit(EXIT_FAILURE);
    }
    ispent = tm_stopwatch(itimer);
    PRINT_RAW("INIT   Spent %d ms.\n", ispent);

    if (check_flag) {
        vflash *flash = fs->get_flash(fs);
        if (flash->check_addr(flash)) {
            PRINT_ERR("Flash Check Address Failed.\n");
        } else {
            PRINT("Flash Check Address Success.\n");
        }

        if (flash->check_val(flash)) {
            PRINT_ERR("Flash Check Value Failed.\n");
        } else {
            PRINT("Flash Check Value Success.\n");
        }
    }

    if (remove_flag) {
        int rspent = test_remove(fs);
        if (rspent < 0) {
            PRINT_ERR("%s() remove failed.\n", __func__);
        } else {
            PRINT_RAW("REMOVE Spent %d ms.\n", rspent);
        }
    }

    if (mkdir_flag) {
        if (fs->mkdir(fs, dir_name)) {
            PRINT_ERR("%s() mkdir \"%s\" failed.\n", __func__, dir_name);
        }
    }

    if (write_flag) {

        unsigned int ospent = 0;
        unsigned int wspent = 0;
        unsigned int cspent = 0;

        Timer ttimer;
        unsigned int tspent = 0;

        tm_set_ms(&ttimer, 0);
        do {
            ospent = test_open(fs);
            if (ospent < 0) {
                PRINT_ERR("%s() test open failed.\n", __func__);
                break;
            }

            wspent = test_write(fs);
            if (wspent < 0) {
                PRINT_ERR("%s() test write failed.\n", __func__);
            }

            cspent = test_close(fs);
            if (cspent < 0) {
                PRINT_ERR("%s() test close failed.\n", __func__);
                break;
            }

        } while(0);
        tspent = tm_stopwatch(ttimer);

        PRINT_RAW("OPEN   Spent %d ms.\n", ospent);
        PRINT_RAW("WRITE  Spent %d ms.\n", wspent);
        PRINT_RAW("CLOSE  Spent %d ms.\n", cspent);
        PRINT_RAW("TOTAL  Spent %d ms.\n", tspent);
    }

    if (cycle_flag) {

        unsigned int ospent = 0;
        unsigned int wspent = 0;
        unsigned int cspent = 0;
        unsigned int rspent = 0;

        Timer ttimer;
        unsigned int tspent = 0;

        tm_set_ms(&ttimer, 0);

        unsigned int cycle = cycle_count;
        do {

            do {

                PRINT(" ---------------- [ %6d ] ----------------\n", cycle);
                ospent = test_open(fs);
                if (ospent < 0) {
                    PRINT_ERR("%s() test_open failed.\n", __func__);
                    break;
                }

                wspent = test_write(fs);
                if (wspent < 0) {
                    PRINT_ERR("%s() test_write failed.\n", __func__);
                }

                cspent = test_close(fs);
                if (cspent < 0) {
                    PRINT_ERR("%s() test_close failed.\n", __func__);
                    break;
                }

            } while(0);

            if (cycle) {
                rspent = test_remove(fs);
                if (rspent < 0) {
                    PRINT_ERR("%s() test_remove failed.\n", __func__);
                    break;
                }
            }

            PRINT(" ---------------- [ %6s ] ----------------\n", "RESULT");
            PRINT_RAW("OPEN   Spent %d ms.\n", ospent);
            PRINT_RAW("WRITE  Spent %d ms.\n", wspent);
            PRINT_RAW("CLOSE  Spent %d ms.\n", cspent);
            PRINT_RAW("REMOVE Spent %d ms.\n", rspent);

        } while(--cycle);

        tspent = tm_stopwatch(ttimer);
        PRINT(" ---------------- [ CONCULTION ] ----------------\n");
        PRINT_RAW("TOTAL Cycle %d times.\n", cycle_count);
        PRINT_RAW("TOTAL Spent %d ms.\n", tspent);
    }

    if (read_flag) {
        test_show_file(fs);
    }

    if (get_file_flag) {

        unsigned int ospent = 0;
        unsigned int gspent = 0;
        unsigned int cspent = 0;

        Timer ttimer;
        unsigned int tspent = 0;

        tm_set_ms(&ttimer, 0);
        do {

            FILE *fd = fopen(get_file_path, "wb");
            if (!fd) {
                PRINT_ERR("%s() fopen failed : %s\n", __func__, strerror(errno));
                return -1;
            }

            ospent = test_open(fs);
            if (ospent < 0) {
                PRINT_ERR("%s() test open failed.\n", __func__);
                break;
            }

            gspent = test_copy_from_fs(fs, fd);
            if (gspent < 0) {
                PRINT_ERR("%s() test write failed.\n", __func__);
            }

            cspent = test_close(fs);
            if (cspent < 0) {
                PRINT_ERR("%s() test close failed.\n", __func__);
                break;
            }

            fclose(fd);

        } while(0);
        tspent = tm_stopwatch(ttimer);

        PRINT_RAW("OPEN   Spent %d ms.\n", ospent);
        PRINT_RAW("GET    Spent %d ms.\n", gspent);
        PRINT_RAW("CLOSE  Spent %d ms.\n", cspent);
        PRINT_RAW("TOTAL  Spent %d ms.\n", tspent);
    }

    if (put_file_flag) {

        unsigned int ospent = 0;
        unsigned int pspent = 0;
        unsigned int cspent = 0;

        Timer ttimer;
        unsigned int tspent = 0;

        tm_set_ms(&ttimer, 0);
        do {

            FILE *fd = fopen(put_file_path, "r");
            if (!fd) {
                PRINT_ERR("fopen failed : %s\n", strerror(errno));
                break;
            }

            ospent = test_open(fs);
            if (ospent < 0) {
                PRINT_ERR("%s() test open failed.\n", __func__);
                break;
            }

            pspent = test_copy_to_fs(fs, fd);
            if (pspent < 0) {
                PRINT_ERR("%s() test write failed.\n", __func__);
            }

            cspent = test_close(fs);
            if (cspent < 0) {
                PRINT_ERR("%s() test close failed.\n", __func__);
                break;
            }

            fclose(fd);

        } while(0);
        tspent = tm_stopwatch(ttimer);

        PRINT_RAW("OPEN   Spent %d ms.\n", ospent);
        PRINT_RAW("PUT    Spent %d ms.\n", pspent);
        PRINT_RAW("CLOSE  Spent %d ms.\n", cspent);
        PRINT_RAW("TOTAL  Spent %d ms.\n", tspent);

    }

    if (list_flag) {
        PRINT_RAW("--------\nPATH : %s\n", list_path);
        test_list_all(fs, list_path, 0);
        PRINT_RAW("--------\n\n");
    }

    if (dump_flag & 0x01) {
        test_dump_raw(fs);
    }
    if (dump_flag & 0x02) {
        test_dump_read(fs);
    }
    if (dump_flag & 0x04) {
        test_dump_info(fs);
    }

    Timer utimer;
    unsigned int uspent = 0;
    tm_set_ms(&utimer, 0);
    if (type == MYFS_LITTLEFS) {
        lfs_uninit(&fs);
    } else if (type == MYFS_FATFS) {
        fat_uninit(&fs);
    } else {
        PRINT_ERR("Unknown type.\n");
        exit(EXIT_FAILURE);
    }
    uspent = tm_stopwatch(utimer);
    PRINT_RAW("UNINIT Spent %d ms.\n", uspent);

    return 0;
}
