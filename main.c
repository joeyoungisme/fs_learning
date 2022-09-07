#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "timer.h"
#include "misc.h"
#include "myfs.h"

#include "ff.h"

static int get_rand_data(unsigned char *data, int size)
{
    if (!data) return 0;
    if (!size) return 0;
    memset(data, 'A' + (rand() % ('Z' - 'A')), size);
    return size;
}

int myfs_mkdir_example(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    if (fs->mkdir(fs, "00")) {
        PRINT_ERR("%s() fs mkdir 00 failed\n", __func__);
        return -1;
    }
    if (fs->mkdir(fs, "01")) {
        PRINT_ERR("%s() fs mkdir 01 failed\n", __func__);
        return -1;
    }

    return 0;
}

int myfs_dirlist_example(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    myfs_dir dir;
    myfs_file_info info;

    if (fs->opendir(fs, &dir, "/")) {
        PRINT_ERR("%s() fs opendir failed\n", __func__);
        return -1;
    }

    printf("--------\r\nRoot directory:\r\n");
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    for(;;) {

        int rc = fs->readdir(fs, &dir, &info);

        if (fs->type == MYFS_LITTLEFS) {
            if (rc < 0) {
                PRINT_ERR("%s() readdir failed (%d).\n", __func__, rc);
                return -1;
            } else if (!rc) {
                break;
            } else if (info.lfs.type & LFS_TYPE_DIR) {
                printf("[LFS] DIR  %s\r\n", info.lfs.name);
                totalDirs++;
            } else {
                printf("[LFS] FILE %s (%d byte)\r\n", info.lfs.name, info.lfs.size);
                totalFiles++;
            }
        } else if (fs->type == MYFS_FATFS) {
            if (rc) {
                break;
            } else if (info.fat.fname[0] == '\0') {
                break;
            } else if (info.fat.fattrib & AM_DIR) {
                printf("[FAT] DIR  %s\r\n", info.fat.fname);
                totalDirs++;
            } else {
                printf("[FAT] FILE %s (%ld byte)\r\n", info.fat.fname, info.fat.fsize);
                totalFiles++;
            }
        } else {
            break;
        }
    }
    printf("(total: %u dirs, %u files)\r\n--------\r\n", totalDirs, totalFiles);

    if (fs->closedir(fs, &dir)) {
        PRINT_ERR("%s() fs closedir failed\n", __func__);
        return -1;
    }

    return 0;
}
int myfs_write_log_example(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    myfs_file file;

    printf("Writing to log.txt...\r\n");

    int flag = 0;
    if (fs->type == MYFS_LITTLEFS) {
        flag = LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND;
    } else if (fs->type == MYFS_FATFS) {
        flag = FA_OPEN_APPEND | FA_WRITE;
    }

    if (fs->open(fs, &file, "log.txt", flag)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return -1;
    }

    char writeBuff[512];
    int bytesToWrite = get_rand_data(writeBuff, 512);
    // PRINT("myfs write (%d) : \n------\n%s\n------\n", bytesToWrite, writeBuff);
    if (fs->write(fs, &file, writeBuff, bytesToWrite) < 0) {
        PRINT_ERR("%s() write failed.\n", __func__);
        return -1;
    }

    if (fs->close(fs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return -1;
    }

    return 0;
}
int myfs_read_log_example(myfs *fs)
{
    if (!fs) {
        return -1;
    }

    myfs_file file;

    printf("Reading file...\r\n");

    if (fs->open(fs, &file, "log.txt", LFS_O_RDWR | LFS_O_CREAT)) {
        PRINT_ERR("%s() open failed.\n", __func__);
        return -1;
    }

    // printf("```\r\n");
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
        // printf("%s", readBuff);
    }
    // printf("\r\n```\r\n");

    if (fs->close(fs, &file)) {
        PRINT_ERR("%s() close failed.\n", __func__);
        return -1;
    }

    return 0;
}

int myfs_test_procedure(myfs *fs)
{
    if (!fs) { return -1; }

    myfs_mkdir_example(fs);
    myfs_dirlist_example(fs);
    myfs_write_log_example(fs);
    myfs_read_log_example(fs);

    return 0;
}

void lfs_init(myfs **lfs)
{
    PRINT("!! LFS INIT !!\r\n");

    *lfs = myfs_new();
    if (!(*lfs)) {
        PRINT_ERR("%s() (*lfs) new failed\n", __func__);
        return;
    } else if ((*lfs)->init((*lfs), MYFS_LITTLEFS, 8, 32, 512)) {
        PRINT_ERR("%s() (*lfs) init failed\n", __func__);
        return;
    } else if ((*lfs)->mount((*lfs))) {
        PRINT_ERR("%s() (*lfs) mount failed\n", __func__);

        if ((*lfs)->format((*lfs))) {
            PRINT_ERR("%s() (*lfs) format failed\n", __func__);
            return;
        } else if ((*lfs)->mount((*lfs))) {
            PRINT_ERR("%s() lfs mount failed (again).\n", __func__);
            return;
        }
    }
}

void lfs_uninit(myfs **lfs)
{
    PRINT("!! LFS UNINIT !!\r\n");

    if ((*lfs)->unmount((*lfs))) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    (*lfs)->destroy((*lfs));
}

void fat_init(myfs **fatfs)
{
    PRINT("!! FAT INIT !!\r\n");

    uint32_t freeClust = 0;
    uint32_t totalBlocks = 0;
    uint32_t freeBlocks = 0;

    *fatfs = myfs_new();
    if (!(*fatfs)) {
        PRINT_ERR("%s() (*fatfs) new failed\n", __func__);
        return;
    } else if ((*fatfs)->init((*fatfs), MYFS_FATFS, 8, 32, 512)) {
        PRINT_ERR("%s() (*fatfs) init failed\n", __func__);
        return;
    } else if ((*fatfs)->mount((*fatfs))) {
        PRINT_ERR("%s() (*fatfs) mount failed\n", __func__);
        return;
    } else if ((*fatfs)->get_free_clust((*fatfs), &freeClust)) {

        PRINT_ERR("%s() gree free clust failed.\n", __func__);

        if ((*fatfs)->format((*fatfs))) {
            PRINT_ERR("%s() (*fatfs) format failed\n", __func__);
            return;
        } else if ((*fatfs)->get_free_clust((*fatfs), &freeClust)) {
            PRINT_ERR("%s() gree free clust failed (again).\n", __func__);
            return;
        }
    }

    totalBlocks = ((*fatfs)->fat->head->core.n_fatent - 2) * (*fatfs)->fat->head->core.csize;
    freeBlocks = freeClust * (*fatfs)->fat->head->core.csize;

    PRINT("Total blocks: %u (%u Mb)\r\n", totalBlocks, totalBlocks / 2000);
    PRINT("Free blocks: %u (%u Mb)\r\n", freeBlocks, freeBlocks / 2000);

}
void fat_uninit(myfs **fat)
{
    PRINT("!! FAT UNINIT !!\r\n");

    if ((*fat)->unmount((*fat))) {
        PRINT_ERR("%s() unmount failed.\n", __func__);
        return;
    }

    (*fat)->destroy((*fat));

    return;
}

int main(int argc, char *argv[])
{

    Timer lfs_timer;
    unsigned int lfs_init_spent = 0;
    unsigned int lfs_test_spent = 0;
    Timer fat_timer;
    unsigned int fat_init_spent = 0;
    unsigned int fat_test_spent = 0;

    srand(time(NULL));

    myfs *lfs = NULL;
    tm_set_ms(&lfs_timer, 0);
    lfs_init(&lfs);
    lfs_init_spent = tm_stopwatch(lfs_timer);

    do {
        tm_set_ms(&lfs_timer, 0);
        myfs_test_procedure(lfs);
        lfs_test_spent = tm_stopwatch(lfs_timer);
    } while(0);
    lfs_uninit(&lfs);

    myfs *fat = NULL;
    tm_set_ms(&fat_timer, 0);
    fat_init(&fat);
    fat_init_spent = tm_stopwatch(fat_timer);
    do {
        tm_set_ms(&fat_timer, 0);
        myfs_test_procedure(fat);
        fat_test_spent = tm_stopwatch(fat_timer);
    } while(0);
    fat_uninit(&fat);

    printf("Lfs init spent time : %d ms.\n", lfs_init_spent);
    printf("Lfs test spent time : %d ms.\n", lfs_test_spent);

    printf("Fat init spent time : %d ms.\n", fat_init_spent);
    printf("Fat test spent time : %d ms.\n", fat_test_spent);

    return 0;
}
