#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "misc.h"
#include "myfs.h"

int myfs_get_free_clust(myfs *fs, unsigned int *free_clust)
{
    if (!fs || !free_clust) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    } else if (fs->type != MYFS_FATFS) {
        PRINT_ERR("%s() invalid type (only fat support).\n", __func__);
        return -1;
    }

    return fs->fat->get_free_clust(fs->fat, 0, free_clust);
}

vflash *myfs_get_flash(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return NULL;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->get_flash(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->get_flash(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return NULL;
}
int myfs_get_read_cnt(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->get_read_cnt(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->get_read_cnt(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_get_prog_cnt(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->get_prog_cnt(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->get_prog_cnt(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_get_erase_cnt(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->get_erase_cnt(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->get_erase_cnt(fs->lfs, 0);
    }
    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_get_sync_cnt(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->get_sync_cnt(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->get_sync_cnt(fs->lfs, 0);
    }
    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
void myfs_clear_cnt(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        fs->fat->clear_cnt(fs->fat, 0);
        return;
    } else if (fs->type == MYFS_LITTLEFS) {
        fs->lfs->clear_cnt(fs->lfs, 0);
        return;
    }
    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return;
}
int myfs_closedir(myfs *fs, myfs_dir *dir)
{
    if (!fs || !dir) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->closedir(fs->fat, 0, &(dir->fat));
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->closedir(fs->lfs, 0, &(dir->lfs));
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_readdir(myfs *fs, myfs_dir *dir, myfs_file_info *info)
{
    if (!fs || !dir || !info) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->readdir(fs->fat, 0, &(dir->fat), &(info->fat));
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->readdir(fs->lfs, 0, &(dir->lfs), &(info->lfs));
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_opendir(myfs *fs, myfs_dir *dir, char *path)
{
    if (!fs || !dir || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->opendir(fs->fat, 0, &(dir->fat), path);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->opendir(fs->lfs, 0, &(dir->lfs), path);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_save(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->save(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->save(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_remove(myfs *fs, char *path)
{
    if (!fs || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs just support 1 dev
    if (fs->type == MYFS_FATFS) {
        return fs->fat->remove(fs->fat, 0, path);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->remove(fs->lfs, 0, path);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_sync(myfs *fs, myfs_file *fd)
{
    if (!fs || !fd) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->type == MYFS_FATFS) {
        // for now , myfs just support 1 dev
        return fs->fat->sync(fs->fat, 0, &(fd->fat));
    } else if (fs->type == MYFS_LITTLEFS) {
        // for now , myfs just support 1 dev
        return fs->lfs->sync(fs->lfs, 0, &(fd->lfs));
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}
int myfs_read(myfs *fs, myfs_file *fd, unsigned char *data, int size)
{
    if (!fs || !fd || !data) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->type == MYFS_FATFS) {
        // for now , myfs just support 1 dev
        return fs->fat->read(fs->fat, 0, &(fd->fat), data, size);
    } else if (fs->type == MYFS_LITTLEFS) {
        // for now , myfs just support 1 dev
        return fs->lfs->read(fs->lfs, 0, &(fd->lfs), data, size);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_write(myfs *fs, myfs_file *fd, unsigned char *data, int size)
{
    if (!fs || !fd || !data) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->type == MYFS_FATFS) {
        // for now , myfs just support 1 dev
        return fs->fat->write(fs->fat, 0, &(fd->fat), data, size);
    } else if (fs->type == MYFS_LITTLEFS) {
        // for now , myfs just support 1 dev
        return fs->lfs->write(fs->lfs, 0, &(fd->lfs), data, size);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_close(myfs *fs, myfs_file *fd)
{
    if (!fs || !fd) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->type == MYFS_FATFS) {
        // for now , myfs just support 1 dev
        return fs->fat->close(fs->fat, 0, &(fd->fat));
    } else if (fs->type == MYFS_LITTLEFS) {
        // for now , myfs just support 1 dev
        return fs->lfs->close(fs->lfs, 0, &(fd->lfs));
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_open(myfs *fs, myfs_file *fd, char *path, int flag)
{
    if (!fs || !fd || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->type == MYFS_FATFS) {
        // for now , myfs just support 1 dev
        return fs->fat->open(fs->fat, 0, &(fd->fat), path, flag);
    } else if (fs->type == MYFS_LITTLEFS) {
        // for now , myfs just support 1 dev
        return fs->lfs->open(fs->lfs, 0, &(fd->lfs), path, flag);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_mkdir(myfs *fs, char *path)
{
    if (!fs || !path) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    myfs_dir dir;

    if (fs->fat) {
        return fs->fat->mkdir(fs->fat, 0, &(dir.fat), path);
    }

    if (fs->lfs) {
        return fs->lfs->mkdir(fs->lfs, 0, &(dir.lfs), path);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_unmount(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->fat) {
        return fs->fat->unmount(fs->fat, 0);
    }

    if (fs->lfs) {
        return fs->lfs->unmount(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_mount(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->fat) {
        return fs->fat->mount(fs->fat, 0);
    }

    if (fs->lfs) {
        return fs->lfs->mount(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_format(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    // for now , myfs default just use one device ( id 0 )
    if (fs->type == MYFS_FATFS) {
        return fs->fat->format(fs->fat, 0);
    } else if (fs->type == MYFS_LITTLEFS) {
        return fs->lfs->format(fs->lfs, 0);
    }

    PRINT_ERR("%s() invalid myfs type.\n", __func__);
    return -1;
}

int myfs_destroy(myfs *fs)
{
    if (!fs) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (fs->fat) {
        fs->fat->destroy(fs->fat);
        fs->fat = NULL;
    }

    if (fs->lfs) {
        fs->lfs->destroy(fs->lfs);
        fs->lfs = NULL;
    }

    return 0;
}
int myfs_init(myfs *fs, MYFS_TYPE type, int blk, int pag, int uni)
{
    if (!fs || (type >= MYFS_TYPE_AMOUNT)) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    if (type == MYFS_LITTLEFS) {
        fs->lfs = mylfs_new();
        if (!fs->lfs) {
            PRINT_ERR("%s() mylfs new failed\n", __func__);
            return -1;
        } else if (fs->lfs->init(fs->lfs)) {
            PRINT_ERR("%s() mylfs init failed\n", __func__);
            return -1;
        } else if (fs->lfs->new_dev(fs->lfs, blk, pag, uni)) {
            PRINT_ERR("%s() mylfs new dev %d.%d.%d failed\n", __func__, blk, pag, uni);
            return -1;
        }
    } else if (type == MYFS_FATFS) {
        fs->fat = myfat_new();
        if (!fs->fat) {
            PRINT_ERR("%s() myfat new failed\n", __func__);
            return -1;
        } else if (fs->fat->init(fs->fat)) {
            PRINT_ERR("%s() myfat init failed\n", __func__);
            fs->fat->destroy(fs->fat);
            return -1;
        } else if (fs->fat->new_dev(fs->fat, blk, pag, uni)) {
            PRINT_ERR("%s() myfat new dev %d.%d.%d failed\n", __func__, blk, pag, uni);
            fs->fat->destroy(fs->fat);
            return -1;
        }
    }

    fs->type = type;

    return 0;
}

myfs *myfs_new(void)
{
    myfs *newfs = (myfs *)malloc(sizeof(myfs));
    if (!newfs) {
        PRINT_ERR("%s() malloc failed : %s\n", __func__, strerror(errno));
        return NULL;
    }
    memset(newfs, 0, sizeof(myfs));

    // assign method
    newfs->init = myfs_init;
    newfs->destroy = myfs_destroy;

    newfs->format = myfs_format;
    newfs->mount = myfs_mount;
    newfs->unmount = myfs_unmount;

    newfs->mkdir = myfs_mkdir;
    newfs->remove = myfs_remove;
    newfs->read = myfs_read;
    newfs->sync = myfs_sync;
    newfs->write = myfs_write;
    newfs->close = myfs_close;
    newfs->open = myfs_open;

    newfs->closedir = myfs_closedir;
    newfs->readdir = myfs_readdir;
    newfs->opendir = myfs_opendir;

    newfs->get_free_clust = myfs_get_free_clust;

    newfs->get_flash = myfs_get_flash;
    newfs->get_read_cnt = myfs_get_read_cnt;
    newfs->get_prog_cnt = myfs_get_prog_cnt;
    newfs->get_erase_cnt = myfs_get_erase_cnt;
    newfs->get_sync_cnt = myfs_get_sync_cnt;
    newfs->clear_cnt = myfs_clear_cnt;

    return newfs;
}

/* USER CODE END Application */

