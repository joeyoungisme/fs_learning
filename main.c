#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "misc.h"
#include "myfs.h"

#include "ff.h"

static void get_rand_data(unsigned char *data, int size)
{
    if (!data) return;
    if (!size) return;
    memset(data, 'A' + (rand() % ('Z' - 'A')), size);
    return;
}

void lfs_example(void)
{
    PRINT("!! LFS EXAMPLE !!\r\n");

    myfs_dir dir;
    myfs_file file;
    myfs_file_info info;

    myfs *myfs = myfs_new();
    if (!myfs) {
        PRINT_ERR("%s() myfs new failed\n", __func__);
        return;
    } else if (myfs->init(myfs, MYFS_LITTLEFS, 8, 32, 512)) {
        PRINT_ERR("%s() myfs init failed\n", __func__);
        return;
    } else if (myfs->mount(myfs)) {
        PRINT_ERR("%s() myfs mount failed\n", __func__);

        if (myfs->format(myfs)) {
            PRINT_ERR("%s() myfs format failed\n", __func__);
            return;
        } else if (myfs->mount(myfs)) {
            PRINT_ERR("%s() myfs mount failed (again).\n", __func__);
            return;
        }
    }

    if (myfs->mkdir(myfs, "00")) {
        PRINT_ERR("%s() myfs mkdir 00 failed\n", __func__);
    }
    if (myfs->mkdir(myfs, "01")) {
        PRINT_ERR("%s() myfs mkdir 01 failed\n", __func__);
    }

    if (myfs->opendir(myfs, &dir, "/")) {
        PRINT_ERR("%s() myfs opendir failed\n", __func__);
        return;
    }

    printf("--------\r\nRoot directory:\r\n");
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    for(;;) {
        int rc = myfs->readdir(myfs, &dir, &info);
        if (rc < 0) {
            PRINT_ERR("%s() readdir failed (%d).\n", __func__, rc);
            return;
        } else if (!rc) {
            break;
        } else if (info.lfs.type & LFS_TYPE_DIR) {
            printf("  DIR  %s\r\n", info.lfs.name);
            totalDirs++;
        } else {
            printf("  FILE %s (%d byte)\r\n", info.lfs.name, info.lfs.size);
            totalFiles++;
        }
    }
    printf("(total: %u dirs, %u files)\r\n--------\r\n", totalDirs, totalFiles);

    if (myfs->closedir(myfs, &dir)) {
        PRINT_ERR("%s() myfs closedir failed\n", __func__);
        return;
    }

    printf("Writing to log.txt...\r\n");

    char writeBuff[512];
    snprintf(writeBuff, sizeof(writeBuff), "@..@ I'm Joeyoung ~ HaHaHa\r\n");

    if (myfs->open(myfs, &file, "log.txt", LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return;
    }

    unsigned int bytesToWrite = strlen(writeBuff);
    PRINT("myfs write (%d) : \n------\n%s\n------\n", bytesToWrite, writeBuff);
    int res = myfs->write(myfs, &file, writeBuff, bytesToWrite);
    if (res < 0) {
    // if (myfs->write(myfs, &file, writeBuff, bytesToWrite) < 0) {
        PRINT_ERR("%s() write failed.\n", __func__);
        return;
    }
    PRINT("myfs write res %d \n", res);

    if (myfs->close(myfs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return;
    }

    printf("Reading file...\r\n");

    if (myfs->open(myfs, &file, "log.txt", LFS_O_RDWR | LFS_O_CREAT)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return;
    }

    printf("```\r\n");
    for(;;)
    {
        char readBuff[512];
        int rlen = myfs->read(myfs, &file, readBuff, sizeof(readBuff)-1);
        if (rlen < 0) {
            PRINT_ERR("%s() read failed.\n", __func__);
            return;
        } else if (!rlen) {
            break;
        }
        readBuff[rlen] = '\0';
        printf("%s", readBuff);
    }
    printf("```\r\n");

    if (myfs->close(myfs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return;
    }

    if (myfs->unmount(myfs)) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    printf("Done!\r\n");

    myfs->destroy(myfs);
}

void fat_example(void)
{
    PRINT("!! FAT EXAMPLE !!\r\n");

    myfs_dir dir;
    myfs_file file;
    myfs_file_info info;
    uint32_t freeClust = 0;
    uint32_t totalBlocks = 0;
    uint32_t freeBlocks = 0;

    myfs *myfs = myfs_new();
    if (!myfs) {
        PRINT_ERR("%s() myfs new failed\n", __func__);
        return;
    } else if (myfs->init(myfs, MYFS_FATFS, 8, 32, 512)) {
        PRINT_ERR("%s() myfs init failed\n", __func__);
        return;
    } else if (myfs->mount(myfs)) {
        PRINT_ERR("%s() myfs mount failed\n", __func__);
        return;
    }

    if (myfs->get_free_clust(myfs, &freeClust)) {

        PRINT_ERR("%s() gree free clust failed.\n", __func__);

        if (myfs->format(myfs)) {
            PRINT_ERR("%s() myfs format failed\n", __func__);
            return;
        } else if (myfs->get_free_clust(myfs, &freeClust)) {
            PRINT_ERR("%s() gree free clust failed (again).\n", __func__);
            return;
        }
    }

    totalBlocks = (myfs->fat->head->core.n_fatent - 2) * myfs->fat->head->core.csize;
    freeBlocks = freeClust * myfs->fat->head->core.csize;

    printf("Total blocks: %u (%u Mb)\r\n", totalBlocks, totalBlocks / 2000);
    printf("Free blocks: %u (%u Mb)\r\n", freeBlocks, freeBlocks / 2000);

    if (myfs->opendir(myfs, &dir, "/")) {
        PRINT_ERR("%s() myfs opendir failed\n", __func__);
        return;
    }

    printf("--------\r\nRoot directory:\r\n");
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    for(;;) {
        if (myfs->readdir(myfs, &dir, &info)) {
            break;
        } else if (info.fat.fname[0] == '\0') {
            break;
        } else if (info.fat.fattrib & AM_DIR) {
            printf("  DIR  %s\r\n", info.fat.fname);
            totalDirs++;
        } else {
            printf("  FILE %s\r\n", info.fat.fname);
            totalFiles++;
        }
    }
    printf("(total: %u dirs, %u files)\r\n--------\r\n", totalDirs, totalFiles);

    if (myfs->closedir(myfs, &dir)) {
        PRINT_ERR("%s() myfs closedir failed\n", __func__);
        return;
    }


    printf("Writing to log.txt...\r\n");

    char writeBuff[512];
    int off = snprintf(writeBuff, sizeof(writeBuff), "Total blocks: %u (%u Mb); Free blocks: %u (%u Mb)\r\n",
              totalBlocks, totalBlocks / 2000,
              freeBlocks, freeBlocks / 2000);
    snprintf(writeBuff + off, sizeof(writeBuff) - off, "@..@ I'm Joeyoung ~ HaHaHa\r\n");

    if (myfs->open(myfs, &file, "log.txt", FA_OPEN_APPEND | FA_WRITE)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return;
    }

    unsigned int bytesToWrite = strlen(writeBuff);
    PRINT("myfs write (%d) : \n------\n%s\n------\n", bytesToWrite, writeBuff);
    int res = myfs->write(myfs, &file, writeBuff, bytesToWrite);
    if (res < 0) {
    // if (myfs->write(myfs, &file, writeBuff, bytesToWrite) < 0) {
        PRINT_ERR("%s() write failed.\n", __func__);
        return;
    }
    PRINT("myfs write res %d \n", res);

    if (myfs->close(myfs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return;
    }

    printf("Reading file...\r\n");

    if (myfs->open(myfs, &file, "log.txt", FA_READ)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return;
    }

    char readBuff[512];
    int rlen = myfs->read(myfs, &file, readBuff, sizeof(readBuff)-1);
    if (rlen < 0) {
        PRINT_ERR("%s() read failed.\n", __func__);
        return;
    }
    readBuff[rlen] = '\0';
    printf("```\r\n%s\r\n```\r\n", readBuff);

    if (myfs->close(myfs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return;
    }

    if (myfs->unmount(myfs)) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    printf("Done!\r\n");

    myfs->destroy(myfs);

    return;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    // fat_example();
    lfs_example();

    return 0;
}
