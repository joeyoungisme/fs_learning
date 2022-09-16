#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "misc.h"
#include "myfs.h"

static int lfs_read(const struct lfs_config *cfg, lfs_block_t blk_num, lfs_off_t off, void *buffer, lfs_size_t size)
{
    mylfs_dev *dev = container_of(cfg, mylfs_dev, config);
    dev->read_cnt++;
    vflash *flash = dev->flash;

    if (!buffer || !size) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if (!flash) {
        PRINT_ERR("flash is NULL\n");
        return -99;
    }

    vblock *block = flash->get_block(flash, blk_num);
    if (!block) {
        PRINT_ERR("get block failed\n");
        return -100;
    }

    if (block->read(block, off, buffer, size)) {
        PRINT_ERR("block write failed\n");
        return -101;
    }

    return 0;
}
static int lfs_prog(const struct lfs_config *cfg, lfs_block_t blk_num, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    mylfs_dev *dev = container_of(cfg, mylfs_dev, config);
    dev->prog_cnt++;
    vflash *flash = dev->flash;

    if (!buffer || !size) {
        PRINT_ERR("invalid args\n");
        return -1;
    } else if (!flash) {
        PRINT_ERR("flash is NULL\n");
        return -99;
    }

    vblock *block = flash->get_block(flash, blk_num);
    if (!block) {
        PRINT_ERR("get block failed\n");
        return -100;
    }

#ifdef STEP_BY_STEP
    // jprintf("[blk_num %d] [unit off %d] [size %d] [%02X ... %02X]\n", blk_num, off / 4, size, ((unsigned char *)buffer)[0], ((unsigned char *)buffer)[size - 1]);
    int page_num = off / flash->page_size(flash);
    PRINT("[PROG] [blk_num %d] [page %d] [size %d] [%02X ... %02X]\n", blk_num, page_num, size, ((unsigned char *)buffer)[0], ((unsigned char *)buffer)[size - 1]);

#ifdef SHOW_DETAIL
    vpage *page = block->get_page(block, page_num);
    if (page) {
        vpage *wpage = page->dupc(page);
        wpage->write(wpage, 0, buffer, size);
        block->wpage_rdump(block, page_num, wpage, 8);
        wpage->destroy(wpage);
    }
#endif

    // block->rdump(block, 8);
#endif

    if (block->write(block, off, buffer, size)) {
        PRINT_ERR("block write failed\n");
        return -101;
    }

    if (flash->fblock_update(flash, blk_num, off, size)) {
        PRINT_ERR("%s() fblock_update blk %d , off %d , size %d failed.\n", __func__, blk_num, off, size);
    }

#ifdef SLOWLY
    sleep(1);
#endif

    // block->rdump(block, 8);

    return 0;
}
static int lfs_erase(const struct lfs_config *cfg, lfs_block_t blk_num)
{
    mylfs_dev *dev = container_of(cfg, mylfs_dev, config);
    dev->erase_cnt++;
    vflash *flash = dev->flash;

    if (!flash) {
        PRINT_ERR("flash is NULL\n");
        return -99;
    }

    vblock *block = flash->get_block(flash, blk_num);
    if (!block) {
        PRINT_ERR("get block failed\n");
        return -100;
    }

#ifdef STEP_BY_STEP
    PRINT("[ERAS] [blk_num %d]\n", blk_num);
#ifdef SHOW_DETAIL
    block->rdump(block, 8);
#endif
#endif

    if (block->erase(block)) {
        PRINT_ERR("block erase failed\n");
        return -101;
    }

    flash->fblock_update(flash, blk_num, 0, flash->block_size(flash));

#ifdef SLOWLY
    sleep(1);
#endif

    return 0;
}
static int lfs_sync(const struct lfs_config *cfg)
{
    mylfs_dev *dev = container_of(cfg, mylfs_dev, config);
    dev->sync_cnt++;

    return 0;
}

// ---- 

//  void myfs_list(myfs *fs, char *path)
//  {
//      if (!fs || !path) {
//          jprintf("invalid args\n");
//          return;
//      }

//      lfs_t *lfs = fs->lfs;
//      lfs_dir_t dir;

//      int res = lfs_dir_open(lfs, &dir, path);
//      if (res < 0) {
//          printf("dir open failed : %s\n", path);
//          return;
//      }

//      res = lfs_dir_rewind(lfs, &dir);
//      if (res < 0) {
//          lfs_dir_close(lfs, &dir);
//          printf("dir rewind failed.\n");
//          return;
//      }

//      struct lfs_info info = {0};

//      while(1)
//      {
//          res = lfs_dir_read(lfs, &dir, &info);
//          if (!res) {
//              break;
//          } else if (res < 0) {
//              jprintf("dir read failed.\n");
//              break;
//          }

//          if (info.type == LFS_TYPE_DIR) {

//              if (!strncmp(info.name, ".", sizeof(info.name))) {
//              } else if (!strncmp(info.name, "..", sizeof(info.name))) {
//              } else {
//                  // sub_path greater than 257 , prevent warning.
//                  char sub_path[512] = {0};
//                  snprintf(sub_path, 512, "%s%s/", path, info.name);
//                  fs->list(fs, sub_path);
//              }
//              continue;
//          }

//          // LFS_TYPE_REG
//          printf("%s%s ( type %X , size %d )\n", path, info.name, info.type, info.size);
//      }

//      lfs_dir_close(lfs, &dir);
//      return;
//  }

// --- new version mylfs dev

int mylfs_dev_save(mylfs_dev *dev, char *file_name)
{
    if (!dev || !file_name) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (!dev->flash) {
        PRINT_ERR("%s() flash no init.\n", __func__);
        return -1;
    }

    if (dev->flash->fexport(dev->flash, file_name)) {
        PRINT_ERR("%s() lfs %d : flash export failed.\n", __func__, dev->id);
        return -1;
    }

    return 0;
}

int mylfs_dev_init(mylfs_dev *dev, int blk, int pag, int uni)
{
    if (!dev) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // -- vflash
    char file_name[64] = {0};
    snprintf(file_name, 64, "lfs%d.config", dev->id);

    vflash *flash = vflash_new();
    if (!flash) {
        PRINT_ERR("vflash new failed.\n");
        dev->destroy(dev);
        return -1;
    } else if (!flash->fimport(flash, file_name)) {
        PRINT("vflash import success.\n");
    } else if (flash->init(flash, blk, pag, uni)) {
        PRINT_ERR("vflash init failed.\n");
        dev->destroy(dev);
        return -1;
    }

    dev->flash = flash;

    // -- lfs config
    dev->config.read = lfs_read;
    dev->config.prog = lfs_prog;
    dev->config.erase = lfs_erase;
    dev->config.sync = lfs_sync;
    dev->config.read_size = flash->page_size(flash);
    dev->config.prog_size =  flash->page_size(flash);
    // dev->config.prog_size =  flash->page_size(flash) / 4;
    dev->config.block_size = flash->block_size(flash);
    dev->config.block_count = flash->vblock_amt;
    dev->config.cache_size = flash->page_size(flash);
    dev->config.lookahead_size = flash->page_size(flash);
    dev->config.block_cycles = flash->page_size(flash);

    PRINT("lfs %d : flash size : %d byte\n", dev->id, dev->flash->size(dev->flash));
    PRINT("lfs %d : block size : %d byte , %d block/flash\n", dev->id, dev->flash->block_size(dev->flash), dev->flash->vblock_amt);
    PRINT("lfs %d : page  size : %d byte , %d page/block\n", dev->id, dev->flash->page_size(dev->flash), dev->flash->vpage_amt);

    dev->save(dev, file_name);

    return 0;
}
int mylfs_dev_destroy(mylfs_dev *dev)
{
    if (!dev) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }


    return 0;
}

mylfs_dev *mylfs_dev_new(void)
{
    mylfs_dev *new_dev = (mylfs_dev *)malloc(sizeof(mylfs_dev));
    if (!new_dev) {
        PRINT_ERR("%s() mylfs dev new malloc failed : %s\n", __func__, strerror(errno));
        return NULL;
    }
    memset(new_dev, 0, sizeof(mylfs_dev));

    new_dev->init = mylfs_dev_init;
    new_dev->destroy = mylfs_dev_destroy;

    new_dev->save = mylfs_dev_save;

    return new_dev;
}

// --- new version

static int is_valid_id(mylfs *lfs, int id)
{
    if (!lfs || !lfs->head) {
        return 0;
    }

    mylfs_dev *dev = lfs->head;
    while(dev) {
        if (dev->id == id) {
            return 1;
        }
        dev = dev->next;
    }

    return 0;
}

static mylfs_dev *get_dev_by_id(mylfs *lfs, int id)
{
    if (!lfs || !lfs->head) {
        return NULL;
    }

    mylfs_dev *dev = lfs->head;
    while(dev) {
        if (dev->id == id) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

void mylfs_clear_cnt(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
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
int mylfs_get_sync_cnt(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->sync_cnt;
}
int mylfs_get_erase_cnt(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->erase_cnt;
}
int mylfs_get_prog_cnt(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->prog_cnt;
}
int mylfs_get_read_cnt(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return dev->read_cnt;
}
vflash *mylfs_get_flash(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return NULL;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return NULL;
    }

    return dev->flash;
}

int mylfs_closedir(mylfs *lfs, int dev_id, lfs_dir_t *dir)
{
    if (!lfs || !dir) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_dir_close(&(dev->core), dir);
}
int mylfs_readdir(mylfs *lfs, int dev_id, lfs_dir_t *dir, struct lfs_info *info)
{
    if (!lfs || !dir || !info) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_dir_read(&(dev->core), dir, info);
}
int mylfs_opendir(mylfs *lfs, int dev_id, lfs_dir_t *dir, char *path)
{
    if (!lfs || !dir || !path) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_dir_open(&(dev->core), dir, path);
}
int mylfs_format(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    if (lfs_format(&(dev->core), &(dev->config))) {
        PRINT_ERR("%s() lfs %d : lfs_format failed\n", __func__, dev->id);
        return -1;
    }

    return 0;
}
int mylfs_mount(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    int res = lfs_mount(&(dev->core), &(dev->config));
    if (res < 0) {
        PRINT_ERR("%s() lfs %d : mount err : %d\n", __func__, dev_id, res);
        return -1;
    }

    return 0;
}
int mylfs_unmount(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    int res = lfs_unmount(&(dev->core));
    if (res < 0) {
        PRINT_ERR("%s() lfs %d : lfs_unmount() failed res = %d\n", __func__, dev->id, res);
        return -1;
    }

    return 0;
}
int mylfs_mkdir(mylfs *lfs, int dev_id, lfs_dir_t *dir, char *path)
{
    if (!lfs || !dir || !path) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    int res = lfs_mkdir(&(dev->core), path);
    if (res < 0) {
        PRINT_ERR("%s() lfs %d : lfs_mkdir() failed res = %d\n", __func__, dev->id, res);
        return -1;
    }

    return 0;
}
int mylfs_open(mylfs *lfs, int dev_id, lfs_file_t *file, char *path, int flag)
{
    if (!lfs || !file || !path) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_file_open(&(dev->core), file, path, flag);
}
int mylfs_close(mylfs *lfs, int dev_id, lfs_file_t *file)
{
    if (!lfs || !file) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    //  if (lfs_file_sync(&(dev->core), file)) {
    //      PRINT_ERR("%s() lfs file sync failed.\n", __func__);
    //      return -1;
    //  }

    if (lfs_file_close(&(dev->core), file)) {
        PRINT_ERR("%s() lfs file close failed.\n", __func__);
        return -1;
    }

    return 0;
}
int mylfs_sync(mylfs *lfs, int dev_id, lfs_file_t *file)
{
    if (!lfs || !file) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_file_sync(&(dev->core), file);
}
int mylfs_write(mylfs *lfs, int dev_id, lfs_file_t *file, unsigned char *data, int len)
{
    if (!lfs || !file || !data) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_file_write(&(dev->core), file, data, len);
}
int mylfs_read(mylfs *lfs, int dev_id, lfs_file_t *file, unsigned char *data, int len)
{
    if (!lfs || !file || !data) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_file_read(&(dev->core), file, data, len);
}
int mylfs_remove(mylfs *lfs, int dev_id, char *path)
{
    if (!lfs || !path) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    return lfs_remove(&(dev->core), path);
}
int mylfs_save(mylfs *lfs, int dev_id)
{
    if (!lfs) {
        PRINT_ERR("%s() invliad args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = get_dev_by_id(lfs, dev_id);
    if (!dev) {
        PRINT_ERR("%s() invalid dev id.\n", __func__);
        return -1;
    }

    char file_name[64] = {0};
    snprintf(file_name, 64, "lfs%d.config", dev->id);

    if (dev->save(dev, file_name)) {
        PRINT_ERR("%s() lfs %d : save failed.\n", __func__, dev_id);
        return -1;
    }

    return 0;
}

int mylfs_init(mylfs *mylfs)
{
    if (!mylfs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    return 0;
}
int mylfs_destroy(mylfs *mylfs)
{
    if (!mylfs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = mylfs->head;
    while(dev) {
        mylfs_dev *next = dev->next;
        dev->destroy(dev);
        dev = next;
    }
    mylfs->head = NULL;

    free(mylfs);

    return 0;
}
int mylfs_list(mylfs *mylfs)
{
    if (!mylfs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (!mylfs->head) {
        PRINT_ERR("%s() no existed device.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = mylfs->head;
    while(dev) {
        // show dev info , like ... root path
        dev = dev->next;
    }

    return 0;
}
int mylfs_new_device(mylfs *mylfs, int blk, int pag, int uni)
{
    if (!mylfs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    mylfs_dev *dev = mylfs_dev_new();
    if (!dev) {
        PRINT_ERR("%s() mylfs dev new failed.\n", __func__);
        return -1;
    } else if (dev->init(dev, blk, pag, uni)) {
        PRINT_ERR("%s() mylfs dev init failed\n", __func__);
        return -1;
    }

    if (!mylfs->head) {
        // first dev
        mylfs->head = dev;
        return 0;
    }

    mylfs_dev *last_dev = mylfs->head;

    while(last_dev->next) { last_dev = last_dev->next; }

    dev->id = last_dev->id + 1;
    last_dev->next = dev;

    return 0;
}
int mylfs_rm_device(mylfs *mylfs, int id)
{
    if (!mylfs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (mylfs->head->id == id) {
        mylfs_dev *rm_dev = mylfs->head;
        mylfs->head = rm_dev->next;
        rm_dev->destroy(rm_dev);
        return 0;
    }

    mylfs_dev *parent = mylfs->head;
    mylfs_dev *child = mylfs->head->next;

    while(child) {
        if (child->id == id) {
            parent->next = child->next;
            child->destroy(child);
            return 0;
        }
        parent = child;
        child = parent->next;
    }

    PRINT_ERR("%s() remove lfs device id %d not found.\n", __func__, id);
    return -1;
}

mylfs *mylfs_new(void)
{
    mylfs *new_lfs = (mylfs *)malloc(sizeof(mylfs));
    if (!new_lfs) {
        PRINT_ERR("%s() mylfs new malloc failed : %s\n", __func__, strerror(errno));
        return NULL;
    }
    memset(new_lfs, 0, sizeof(mylfs));

    new_lfs->init = mylfs_init;
    new_lfs->destroy = mylfs_destroy;
    new_lfs->list = mylfs_list;
    new_lfs->new_dev = mylfs_new_device;
    new_lfs->rm_dev = mylfs_rm_device;

    new_lfs->format = mylfs_format;
    new_lfs->mount = mylfs_mount;
    new_lfs->unmount = mylfs_unmount;
    new_lfs->mkdir = mylfs_mkdir;
    new_lfs->opendir = mylfs_opendir;
    new_lfs->readdir = mylfs_readdir;
    new_lfs->closedir = mylfs_closedir;
    new_lfs->opendir = mylfs_opendir;
    new_lfs->open = mylfs_open;
    new_lfs->close = mylfs_close;
    new_lfs->write = mylfs_write;
    new_lfs->read = mylfs_read;
    new_lfs->sync = mylfs_sync;
    new_lfs->remove = mylfs_remove;
    new_lfs->save = mylfs_save;

    new_lfs->clear_cnt = mylfs_clear_cnt;
    new_lfs->get_sync_cnt = mylfs_get_sync_cnt;
    new_lfs->get_erase_cnt = mylfs_get_erase_cnt;
    new_lfs->get_prog_cnt = mylfs_get_prog_cnt;
    new_lfs->get_read_cnt = mylfs_get_read_cnt;
    new_lfs->get_flash = mylfs_get_flash;


    return new_lfs;
}
