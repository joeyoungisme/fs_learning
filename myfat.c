#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "misc.h"
#include "myfat.h"

#ifdef GGR_FATFS
#include "ggr_fatfs_diskio.h"
#endif

static Timer myfat_timer;
static myfat_dev *head;

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
    tm_from_sys(&myfat_timer);
    DWORD fattime;
    tm_to_fattime(&myfat_timer, &fattime);
    return fattime;
}

myfat_dev *find_fat_by_id(BYTE id)
{
    if (!head) {
        return NULL;
    }

    myfat_dev *dev = head;

    while(dev) {
        if (dev->id == id) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS fat_disk_init(BYTE pdrv)
{
    myfat_dev *dev = find_fat_by_id(pdrv);
    if (!dev) {
        return RES_PARERR;
    }

#ifdef GGR_FATFS
    return ggr_fatfs_initialize(pdrv);
#else
    return RES_OK;
#endif
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS fat_disk_status(BYTE pdrv)
{
    myfat_dev *dev = find_fat_by_id(pdrv);
    if (!dev) {
        return RES_PARERR;
    }

#ifdef GGR_FATFS
    return ggr_fatfs_status(pdrv);
#else
    return RES_OK;
#endif
}

static int fat_block_sector_amount(vflash *flash)
{
    return (flash->block_size(flash) / _MAX_SS);
}
static int fat_block_first_sector(vflash *flash, int block)
{
    return (flash->block_size(flash) * block) / _MAX_SS;
}
static int fat_sector_to_block(vflash *flash, DWORD sector)
{
    unsigned long sec_per_blk = flash->block_size(flash) / _MAX_SS;

    return sector / sec_per_blk;
}
static int fat_sector_to_page(vflash *flash, DWORD sector)
{
    unsigned long sec_per_blk = flash->block_size(flash) / _MAX_SS;
    unsigned long sec_per_pag = flash->page_size(flash) / _MAX_SS;

    return (sector % sec_per_blk) / sec_per_pag;
}
static int fat_sector_to_page_off(vflash *flash, DWORD sector)
{
    unsigned long sec_per_blk = flash->block_size(flash) / _MAX_SS;
    unsigned long sec_per_pag = flash->page_size(flash) / _MAX_SS;

    return ((sector % sec_per_blk) % sec_per_pag) * _MAX_SS;
}
static int fat_sector_to_block_off(vflash *flash, DWORD sector)
{
    int page = fat_sector_to_page(flash, sector);
    int page_off = fat_sector_to_page_off(flash, sector);

    return (page * flash->page_size(flash)) + page_off;
}
static int fat_over_write_detect(vflash *flash, DWORD sector)
{
    int bidx = fat_sector_to_block(flash, sector);
    int pidx = fat_sector_to_page(flash, sector);

    vblock *block = flash->get_block(flash, bidx);
    vpage *page = block->get_page(block, pidx);

    for (int idx = 0; idx < page->vunit_amt; ++idx) {
        if (page->vunit[idx].used) return 1;
    }

    return 0;
}
static int fat_erase_sector(vflash *flash, DWORD sector, int count)
{
    if (!flash || !count) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if ((sector + count) > (flash->size(flash) / _MAX_SS)) {
        PRINT_ERR("%s() invalid sector.\n", __func__);
        return -1;
    }

    int blk_size = flash->block_size(flash);
    unsigned char *block_buff = (unsigned char *)malloc(blk_size);
    if (!block_buff) {
        PRINT_ERR("%s() malloc failed : %s.\n", __func__, strerror(errno));
        return -1;
    }
    memset(block_buff, 0, blk_size);

    // | first part  |       middle part         |  last part   |
    // | <- block -> | <- block -> | <- block -> | <- block ->  |
    //         □□□□□□|□□□□□□□□□□□□□|□□□□□□□□□□□□□|□□□□□□ 
    //  ■■■■■■■                                         ■■■■■■■■
    //  ^(write back sector)                            ^(write back sector)

    int sector_per_block = flash->block_size(flash) / _MAX_SS;

    // first part
    //
    // | <- block -> |
    // |□□□□□□□□□□□□□| <- erase block
    //  ■■■■■■   ■■■■  < count = 3
    //  ■■■■■■         < count > sector per block
    //         ■■■■■■  < count > sector per block
    //
    while (count) {

        int blk = fat_sector_to_block(flash, sector);
        vblock *block = flash->get_block(flash, blk);
        if (!block) {
            PRINT_ERR("%s() flash get block %d failed.\n", __func__, blk);
            return -1;
        } else if (block->read(block, 0, block_buff, blk_size)) {
            PRINT_ERR("%s() block %d read failed.\n", __func__, blk);
            free(block_buff);
            return -1;
        } else if (block->erase(block)) {
            PRINT_ERR("%s() block %d erase failed.\n", __func__, blk);
            free(block_buff);
            return -1;
        }

        int fsector = fat_block_first_sector(flash, blk);
        int bsector = sector % sector_per_block;
        int bcount = sector_per_block - bsector;
        if (count < bcount) {
            bcount = count;
        }

#ifdef STEP_BY_STEP
        PRINT("FAT BLOCK %d ERASE SECTOR [%ld - %ld] [%ld - %ld] [%ld - %ld].\n", blk, fsector, bsector, bsector, bsector + bcount, bsector + bcount, sector_per_block);
#endif

        if (bsector) {
            int wsize = bsector * _MAX_SS;
            if (block->write(block, 0, block_buff, wsize)) {
                PRINT_ERR("%s() block %d write 0 ~ %d failed.\n", __func__, blk, wsize);
                free(block_buff);
                return -1;
            }
#ifdef STEP_BY_STEP
            PRINT("FAT BLOCK %d WRITE BACK FRONT [%ld - %ld] OK.\n", blk, fsector, bsector);
#endif
        }

        if ((bsector + bcount) % sector_per_block) {
            int woff = (bsector + bcount) * _MAX_SS;
            int wsize = (sector_per_block - (bsector + bcount)) * _MAX_SS;
            if (block->write(block, woff, block_buff + woff, wsize)) {
                PRINT_ERR("%s() block %d write %d ~ %d failed.\n", __func__, blk, woff, wsize);
                free(block_buff);
                return -1;
            }
#ifdef STEP_BY_STEP
            PRINT("FAT BLOCK %d WRITE BACK END [%ld - %ld] OK.\n", blk, bsector + bcount, sector_per_block);
#endif
        }

        count -= bcount;
        sector += bcount;

        flash->fblock_update(flash, blk, 0, flash->block_size(flash));
    }

    free(block_buff);

    return 0;
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT fat_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    myfat_dev *dev = find_fat_by_id(pdrv);
    if (!dev) {
        return RES_PARERR;
    }

#ifdef STEP_BY_STEP
    PRINT("[FAT DISK READ] sector %ld , count %d.\n", sector, count);
#endif

    vflash *flash = dev->flash;
    if (flash->read(flash, sector * _MAX_SS, count * _MAX_SS, buff)) {
        PRINT_ERR("%s() flash read addr %ld , size %d.\n", __func__, sector * _MAX_SS, count * _MAX_SS);
        return RES_ERROR;
    }

    dev->read_cnt++;

    return RES_OK;
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT fat_disk_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    myfat_dev *dev = find_fat_by_id(pdrv);
    if (!dev) {
        return RES_PARERR;
    }

#ifdef STEP_BY_STEP
        PRINT("[FAT DISK WRITE] [sector %ld] [count %d]\n", sector, count);
#endif

    vflash *flash = dev->flash;

    if (fat_over_write_detect(flash, sector)) {
#ifdef STEP_BY_STEP
        PRINT("[FAT DISK WRITE] detect sector %ld over write.\n", sector);
#endif
        if (fat_erase_sector(flash, sector, 1)) {
            PRINT_ERR("%s() fat erase sector %ld failed.\n", __func__, sector);
            return RES_ERROR;
        }
    }

    if (flash->write(flash, sector * _MAX_SS, count * _MAX_SS, buff)) {
        PRINT_ERR("%s() flash write addr %ld , size %d.\n", __func__, sector * _MAX_SS, count * _MAX_SS);
        return RES_ERROR;
    }

    flash->fupdate(flash, sector * _MAX_SS, count * _MAX_SS);

    dev->prog_cnt++;

    return RES_OK;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT fat_disk_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    myfat_dev *dev = find_fat_by_id(pdrv);
    if (!dev) {
        return RES_PARERR;
    }

    if (cmd == CTRL_SYNC) {
        // no need return value , just sync data
    } else if (cmd == GET_SECTOR_COUNT) {
        *(DWORD *)buff = dev->flash->size(dev->flash) / _MAX_SS;
    } else if (cmd == GET_BLOCK_SIZE) {
        *(DWORD *)buff = dev->flash->block_size(dev->flash);
#if _USE_TRIM == 1
    } else if (cmd == CTRL_TRIM) {

        DWORD *dw_point = buff;
        DWORD start = dw_point[0];
        DWORD end = dw_point[1];

#ifdef STEP_BY_STEP
        PRINT("%s() ERASE SECTOR %ld - %ld.\n", __func__, start, end);
#endif
        fat_erase_sector(dev->flash, start, end - start + 1);
#endif
    }

    return RES_OK;
}
#endif /* _USE_IOCTL == 1 */

// ----


int myfat_dev_save(myfat_dev *dev, char *file_name)
{
    if (!dev || !file_name) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (dev->flash->fexport(dev->flash, file_name)) {
        PRINT_ERR("%s() fat %d : fexport failed.\n", __func__, dev->id);
        return -1;
    }

    return 0;
}
int myfat_dev_change(myfat_dev *dev)
{
    if (!dev) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

#if _VOLUMES >= 2
    FRESULT res = f_chdrive(dev->path);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_chrdrive faild.\n", __func__, res);
        return -1;
    }
#endif

    return 0;
}

int myfat_dev_init(myfat_dev *dev, int blk, int pag, int unit)
{
    if (!dev) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    dev->config = (Diskio_drvTypeDef *)malloc(sizeof(Diskio_drvTypeDef));
    if (!dev->config) {
        PRINT_ERR("%s() malloc failed : %s\n", __func__, strerror(errno));
        dev->destroy(dev);
        return -1;
    }
    memset(dev->config, 0, sizeof(Diskio_drvTypeDef));

    // assign dev method
    dev->config->disk_initialize = fat_disk_init;
    dev->config->disk_status = fat_disk_status;
    dev->config->disk_read = fat_disk_read;
#if _USE_WRITE == 1
    dev->config->disk_write = fat_disk_write;
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
    dev->config->disk_ioctl = fat_disk_ioctl;
#endif /* _USE_IOCTL == 1 */

    if (FATFS_LinkDriver(dev->config, dev->path)) {
        PRINT("FATFS_LinkDriver failed.\n");
        dev->destroy(dev);
        return -1;
    }
    PRINT("FATFS LinkDriver \"%s\"\n", dev->path);

    // init parameter
    dev->id = dev->path[0] - '0';
    dev->state = STA_NOINIT;

    // ---- vflash
    char file_name[64] = {0};
    snprintf(file_name, 64, "fat%d.config", dev->id);

    dev->flash = vflash_new();
    if (!dev->flash) {
        PRINT_ERR("vflash new failed.\n");
        dev->destroy(dev);
        return -1;
    } else if (!dev->flash->fimport(dev->flash, file_name)) {
        PRINT("vflash import success.\n");
    } else if (dev->flash->init(dev->flash, blk, pag, unit)) {
        PRINT_ERR("vflash init failed.\n");
        dev->destroy(dev);
        return -1;
    }

    if (dev->flash->block_size(dev->flash) <= 32768) {
        PRINT_ERR("vflash block size too less.\n");
        PRINT_ERR("block %d byte < 32768 byte\n", dev->flash->block_size(dev->flash));
        dev->destroy(dev);
        return -1;
    }

    dev->clus_size = 0x800;


    PRINT_RAW("fat %d : flash ( %d bytes ) , block ( %d bytes ) , page ( %d bytes ) , unit ( %ld bytes ).\n",
              dev->id, dev->flash->size(dev->flash), dev->flash->block_size(dev->flash), dev->flash->page_size(dev->flash), sizeof(VUNIT_TYPE));

    PRINT_RAW("fat %d : %ld bytes / %d units / %d pages / %d blocks / flash.\n",
              dev->id, sizeof(VUNIT_TYPE), dev->flash->vunit_amt, dev->flash->vpage_amt, dev->flash->vblock_amt);

    PRINT_RAW("fat %d : cluster ( %d bytes ) , sector ( %d bytes ).\n", dev->id, dev->clus_size, _MAX_SS);
    PRINT_RAW("fat %d : %d (sector/cluster) , %d (sector/page) , %d (sector/block) , %d (sector/flash).\n",
              dev->id, _MAX_SS/dev->clus_size, _MAX_SS/dev->flash->page_size(dev->flash), _MAX_SS/dev->flash->block_size(dev->flash), _MAX_SS/dev->flash->size(dev->flash));

    dev->save(dev, file_name);

    return 0;
}

int myfat_dev_destroy(myfat_dev *dev)
{
    if (!dev) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (strlen(dev->path)) {
        if (FATFS_UnLinkDriver(dev->path)) {
            PRINT_ERR("%s() UnLinkDriver failed\n", __func__);
        }
    }

    if (dev->config) {
        free(dev->config);
        dev->config = NULL;
    }

    if (dev->flash) {
        dev->flash->destroy(dev->flash);
        dev->flash = NULL;
    }

    free(dev);

    return 0;
}

myfat_dev *myfat_dev_new(void)
{
    myfat_dev *new_dev = (myfat_dev *)malloc(sizeof(myfat_dev));
    if (!new_dev) {
        PRINT_ERR("%s() malloc failed , %s\n", __func__, strerror(errno));
        return NULL;
    }
    memset(new_dev, 0, (sizeof(myfat_dev)));

    // assign method
    new_dev->init = myfat_dev_init;
    new_dev->destroy = myfat_dev_destroy;

    new_dev->change = myfat_dev_change;

    new_dev->save = myfat_dev_save;
    
    return new_dev;
}

static int is_valid_id(myfat *fat, int id)
{
    if (!fat || !fat->head) {
        return 0;
    }

    myfat_dev *dev = fat->head;
    while(dev) {
        if (dev->id == id) {
            return 1;
        }
        dev = dev->next;
    }

    return 0;
}
static myfat_dev *get_dev_by_id(myfat *fat, int id)
{
    if (!fat || !fat->head) {
        return NULL;
    }

    myfat_dev *dev = fat->head;
    while(dev) {
        if (dev->id == id) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

// --- fat dev api
int myfat_closedir(myfat *fat, int dev_id, DIR *dir)
{
    if (!fat || !dir) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_closedir(dir);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_closedir failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
int myfat_readdir(myfat *fat, int dev_id, DIR *dir, FILINFO *info)
{
    if (!fat || !dir || !info) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_readdir(dir, info);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_readdir failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
int myfat_opendir(myfat *fat, int dev_id, DIR *dir, char *path)
{
    if (!fat || !dir || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_opendir(dir, path);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_opendir failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
int myfat_format(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }
   
    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }
    
    unsigned char work_buff[2048] = {0};
    FRESULT res = f_mkfs(dev->path, FM_ANY, dev->clus_size, work_buff, 2048);
    if(res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_mkfs() failed, res = %d\n", __func__, dev->id, res);
        return -1;
    }
    
    return 0;
}
int myfat_mount(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    FRESULT res = f_mount(&(dev->core), "", 0);
    if(res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_mount() failed, res = %d\n", __func__, dev->id, res);
        return -1;
    }

    return 0;
}
int myfat_unmount(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }
   
    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    FRESULT res = f_mount(NULL, dev->path, 0);
    if(res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_mount() failed, res = %d\n", __func__, dev->id, res);
        return -1;
    }

    return 0;
}
int myfat_mkdir(myfat *fat, int dev_id, DIR *dir, char *path)
{
    if (!fat || !dir || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_mkdir(path);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_mkdir \"%s\" faild.\n", __func__, res, path);
        return -1;
    }

    return 0;
}
int myfat_open(myfat *fat, int dev_id, FIL *file, char *path, int mode)
{
    if (!fat || !file || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }
    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_open(file, path, mode);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_open \"%s\" failed (%d).\n", __func__, dev_id, path, res);
        return -1;

    }

    return 0;
}
int myfat_close(myfat *fat, int dev_id, FIL *file)
{
    if (!fat || !file) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FRESULT res = f_close(file);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_close failed.\n", __func__, dev_id);
        return -1;

    }

    return 0;
}
int myfat_write(myfat *fat, int dev_id, FIL *file, unsigned char *data, int len)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    // no need to change ?

    UINT wlen = 0;
    FRESULT res = f_write(file, data, len, &wlen);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_write failed.\n", __func__, dev_id);
        return -1;
    } else if (wlen != len) {
        PRINT_ERR("%s() fat %d : write size mismatch failed.\n", __func__, dev_id);
        PRINT_ERR("write len (%d) != expect len (%d).\n", wlen, len);
        return -1;
    }

    return (int)wlen;
}
int myfat_read(myfat *fat, int dev_id, FIL *file, unsigned char *data, int len)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }
   
    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    // no need to change ?

    UINT rlen = 0;
    FRESULT res = f_read(file, data, len, &rlen);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_read failed.\n", __func__, dev_id);
        return -1;
    }

    return (int)rlen;
}
int myfat_sync(myfat *fat, int dev_id, FIL *file)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }
   
    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    // no need to change ?

    UINT rlen = 0;
    FRESULT res = f_sync(file);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_sync failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
int myfat_remove(myfat *fat, int dev_id, char *path)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    /* Delete an existing file or directory */
    FRESULT res = f_unlink(path);
    if (res != FR_OK) {
        PRINT_ERR("%s() fat %d : f_unlink failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
vflash *myfat_get_flash(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return NULL;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return NULL;
    }

    return dev->flash;
}
int myfat_get_read_cnt(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->read_cnt;
}
int myfat_get_prog_cnt(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->prog_cnt;
}
int myfat_get_erase_cnt(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->erase_cnt;
}
int myfat_get_sync_cnt(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->sync_cnt;
}
void myfat_clear_cnt(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return;
    }

    dev->read_cnt = 0;
    dev->prog_cnt = 0;
    dev->erase_cnt = 0;
    dev->sync_cnt = 0;

    return;
}
int myfat_get_free_clust(myfat *fat, int dev_id, unsigned int *free_clust)
{
    if (!fat || !free_clust) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (dev->change(dev)) {
        PRINT_ERR("%s() fat %d : change device failed.\n", __func__, dev_id);
        return -1;
    }

    FATFS *check = NULL;
    FRESULT res = f_getfree(dev->path, (DWORD *)free_clust, &check);
    if ((res != FR_OK) || (&dev->core != check)) {
        PRINT_ERR("%s() fat %d : f_getfree failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}
int myfat_save(myfat *fat, int dev_id)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = get_dev_by_id(fat, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    char file_name[64] = {0};
    snprintf(file_name, 64, "fat%d.config", dev->id);

    if (dev->save(dev, file_name)) {
        PRINT_ERR("%s() fat %d : save failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}


int myfat_list(myfat *fat)
{
    if (!fat) {
    } else if (!fat->head) {
    }
}
int myfat_new_device(myfat *fat, int blk, int pag, int unit)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // new fat dev
    myfat_dev *new_dev = myfat_dev_new();
    if (!new_dev) {
        PRINT_ERR("%s() myfat dev new failed\n", __func__);
        return -1;
    } else if (new_dev->init(new_dev, blk, pag, unit)) {
        PRINT_ERR("%s() myfat dev init failed (%d, %d, %d).\n", __func__, blk, pag, unit);
        return -1;
    }
   
    if (!fat->head) {
        PRINT("First Fat Device Registered.\n");
        fat->head = new_dev;
        head = fat->head;
        return 0;
    }

    // find last dev
    myfat_dev *last = fat->head;
    while(last->next) { last = last->next; }
    last->next = new_dev;

    return 0;
}
int myfat_remove_device(myfat *fat, int dev_id)
{
    if (is_valid_id(fat, dev_id)) {
        // ...
    }
    return 0;
}
int myfat_init(myfat *fat)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    return 0;
}
int myfat_destroy(myfat *fat)
{
    if (!fat) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfat_dev *dev = fat->head;
    while(dev) {
        myfat_dev *next = dev->next;
        dev->destroy(dev);
        dev = next;
    }
    fat->head = NULL;
    head = fat->head;

    return 0;
}


myfat *myfat_new(void)
{
    myfat *fat = (myfat *)malloc(sizeof(myfat));
    if (!fat) {
        PRINT_ERR("%s() malloc failed , %s\n", __func__, strerror(errno));
        return NULL;
    }
    memset(fat, 0, sizeof(myfat));

    // assign method
    fat->init = myfat_init;
    fat->destroy = myfat_destroy;

    fat->list = myfat_list;
    fat->new_dev = myfat_new_device;
    fat->rm_dev = myfat_remove_device;

    fat->format = myfat_format;
    fat->mount = myfat_mount;
    fat->unmount = myfat_unmount;
    fat->mkdir = myfat_mkdir;
    fat->opendir = myfat_opendir;
    fat->readdir = myfat_readdir;
    fat->closedir = myfat_closedir;

    fat->open = myfat_open;
    fat->close = myfat_close;
    fat->write = myfat_write;
    fat->read = myfat_read;
    fat->sync = myfat_sync;
    fat->remove = myfat_remove;
    fat->save = myfat_save;

    fat->get_free_clust = myfat_get_free_clust;

    fat->get_flash = myfat_get_flash;
    fat->get_read_cnt = myfat_get_read_cnt;
    fat->get_prog_cnt = myfat_get_prog_cnt;
    fat->get_erase_cnt = myfat_get_erase_cnt;
    fat->get_sync_cnt = myfat_get_sync_cnt;
    fat->clear_cnt = myfat_clear_cnt;

    return fat;
}
