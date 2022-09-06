#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "myfs.h"

#ifdef XXXXXXXXXX

// joe printf
#define va_args(...)        , ##__VA_ARGS__
#define jprintf(fmt, ...) \
    do { \
        printf("%s [LINE %d] : " fmt, __func__, __LINE__ va_args(__VA_ARGS__)); \
    } while(0)

void example_fs_test_format(myfs *fs) {
    clock_t begin = clock();
    fs->format(fs);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Format elapsed %f sec\n", time_spent);
    return;
}

static void get_rand_data(unsigned char *data, len)
{

}

void example_fs_fill_log_file(myfs *fs, int dir_max, int file_max, int ent_max) {

    int exit_flag = 0;

    for (int dir = 0; dir < dir_max; ++dir) {

        char dir_name[8] = {0};
        snprintf(dir_name, 8, "%d", dir);
        // printf("CREATE DIR : %s\n", dir_name);
        int res = fs->mkdir(fs, dir_name);
        if (res < 0) {
            printf("mkdir err.\n");
        }
    }

    srand(time(NULL));

    for (int file_idx = 0; file_idx < file_max; ++file_idx) {

        for (int dir = 0; dir < dir_max; ++dir) {

            lfs_file_t file;
            char file_name[16] = {0};
            snprintf(file_name, 16, "%d/%02X", dir, file_idx);

            clock_t begin = clock();

            if (fs->open(fs, &file, file_name, LFS_O_RDWR | LFS_O_CREAT)) {
                printf("open err.\n");
                return;
            }

            // every file about 128K
            for (int line = 0; line < ent_max; ++line) {

                char data[128] = {0};
                memset(data, 'A' + (rand() % ('Z' - 'A')), 128);
                // printf("WRITE : %s\n", data);
                int res = fs->write(fs, &file, data, 128);
                if (res < 0) {
                    printf("write %d line failed.\n", line);
                    exit_flag = 1;
                    break;
                }
            }

            // printf("CLOSE FILE : %s\n", file_name);
            fs->close(fs, &file);

            clock_t end = clock();
            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("FILE : %s , elapsed %f sec\n", file_name, time_spent);
        }

        if (exit_flag) {
            break;
        }
    }

    return;
}

void example_bridge_log(myfs *fs) {

    for (int dir = 0; dir < 8; ++dir) {

        char dir_name[8] = {0};
        snprintf(dir_name, 8, "%d", dir);
        printf("CREATE DIR : %s\n", dir_name);
        int res = fs->mkdir(fs, dir_name);
        if (res < 0) {
            printf("mkdir err.\n");
            return;
        }

        srand(time(NULL));

        for (int file_idx = 0; file_idx < 0x20; ++file_idx) {

            lfs_file_t file;
            char file_name[16] = {0};
            snprintf(file_name, 16, "%s/%02X", dir_name, file_idx);

            printf("OPEN FILE : %s\n", file_name);
            if (fs->open(fs, &file, file_name, LFS_O_RDWR | LFS_O_CREAT)) {
                printf("open err.\n");
                return;
            }

            // every file about 128K
            for (int line = 0; line < 1000; ++line) {

                char data[128] = {0};
                memset(data, 'A' + (rand() % ('Z' - 'A')), 128);
                printf("WRITE : %s\n", data);
                int res = fs->write(fs, &file, data, 128);
                if (res < 0) {
                    printf("write %d line failed.\n", line);
                }
            }

            printf("CLOSE FILE : %s\n", file_name);
            fs->close(fs, &file);
        }
    }

    return;
}
void example_fs_create_dir(myfs *fs, char *name)
{
    if (!fs || !name) { return; }

    printf("CREATE DIR : %s\n", name);
    int res = fs->mkdir(fs, name);
    if (res < 0) {
        printf("mkdir err.\n");
    }
    
    return;
}
void example_create_file(myfs *fs, char *name)
{
    if (!fs || !name) { return; }

    lfs_file_t file;

    printf("OPEN FILE : %s\n", name);
    if (fs->open(fs, &file, name, LFS_O_RDWR | LFS_O_CREAT)) {
        printf("open err.\n");
        return;
    }

    printf("CLOSE FILE : %s\n", name);
    fs->close(fs, &file);
    
    return;
}
void example_fs_write_file(myfs *fs, char *name)
{
    if (!fs || !name) { return; }

    srand(time(NULL));

    lfs_file_t file;

    printf("OPEN FILE : %s\n", name);
    if (fs->open(fs, &file, name, LFS_O_RDWR | LFS_O_CREAT)) {
        printf("open err.\n");
        return;
    }

    char data[64] = {0};
    memset(data, 'A' + (rand() % ('Z' - 'A')), 64);
    int wsize = (rand()%63) + 1;
    data[wsize] = '\0';
    printf("WRITE (%d) : %s\n", wsize, data);
    int res = fs->write(fs, &file, data, wsize);
    if (res < 0) {
        printf("write failed.\n");
    }

    printf("CLOSE FILE : %s\n", name);
    fs->close(fs, &file);

    return;
}
void example_fs_read_file(myfs *fs, char *name)
{
    if (!fs || !name) { return; }

    srand(time(NULL));

    lfs_file_t file;

    if (fs->open(fs, &file, name, LFS_O_RDWR | LFS_O_CREAT)) {
        printf("open err.\n");
        return;
    }

    char data[64] = {0};
    int res = fs->read(fs, &file, data, 64);
    if (res < 0) {
        printf("read failed.\n");
    } else {
        printf("read (%d) : %s\n", res, data);
    }

    fs->close(fs, &file);
    
    return;
}
void example_fs_file_list(myfs *fs)
{
    if (!fs) { return; }

    printf(" ---- Dir List ./ ----\n");
    fs->list(fs, "./");
    printf(" ---------------------\n");
    
    return;
}

void example_fs_remove_all(myfs *fs)
{
    printf(" ---- Remove List ./ ----\n");
    fs->remove_all(fs, "./");
    printf(" ---------------------\n");
}

void example_vflash_info(myfs *fs)
{
    if (!fs) { return; }

    vflash *flash = fs->get_flash(fs);

    printf("Virtual Flash Total Size : %d\n", flash->size(flash));
    printf("Virtual Flash Total Block : %d\n", flash->vblock_amt);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->get_block(flash, bidx);

        printf("Virtual Block[%d] Total Size : %d\n", bidx, block->size(block));
        printf("Virtual Block[%d] Total Page : %d\n", bidx, block->vpage_amt);

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->get_page(block, pidx);

            printf("Virtual Page[%d] Total Size : %d\n", pidx, page->size(page));
            printf("Virtual Page[%d] Total Unit : %d\n", pidx, page->vunit_amt);

            vunit *unit = page->vunit;

            printf("Virtual Unit[0] Size : %ld\n", sizeof(unit[0].data));

            // prevent show too many info
            break;
        }

        // prevent show too many info
        break;
    }

    return;
}
void example_vflash_dump_all(myfs *fs)
{
    if (!fs) { return; }

    vflash *flash = fs->get_flash(fs);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->get_block(flash, bidx);

        // every 8 unit next line , readable dump
        printf("\nBlock %d Rdump (%d byte)\n", bidx, block->size(block));
        block->rdump(block, 16);

    }

    return;
}
void example_vflash_block_dump(myfs *fs)
{
    if (!fs) { return; }

    vflash *flash = fs->get_flash(fs);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->get_block(flash, bidx);

        printf("\nBlock %d Dump (%d byte)\n", bidx, block->size(block));
        block->dump(block, 8);

        // every 8 unit next line , readable dump
        printf("\nBlock %d Rdump (%d byte)\n", bidx, block->size(block));
        block->rdump(block, 8);

        // every 8 unit next line
        // [■ (01,01)] : in used unit, erase 1 times, write 1 times
        // [□ (01,00)] : no used unit, erase 1 times, write 0 times
        printf("\nBlock %d Info, ■ in used, □ no used, (erase times, write times)\n", bidx);
        block->info(block, 6);

        // prevent show too many info
        break;
    }

    return;
}
void example_vflash_page_dump(myfs *fs)
{
    if (!fs) { return; }

    vflash *flash = fs->get_flash(fs);

    for (int bidx = 0; bidx < flash->vblock_amt; ++bidx) {

        vblock *block = flash->get_block(flash, bidx);

        // block->dump(block, 8);
        // block->rdump(block, 8);
        // block->info(block, 4);

        for (int pidx = 0; pidx < block->vpage_amt; ++pidx) {

            vpage *page = block->get_page(block, pidx);

            // every 8 unit next line
            printf("\nBlock %d Page %d Dump (%d byte)\n", bidx, pidx, page->size(page));
            page->dump(page, 8);

            // every 8 unit next line , readable dump
            printf("\nBlock %d Page %d Rdump (%d byte)\n", bidx, pidx, page->size(page));
            page->rdump(page, 8);

            // every 8 unit next line
            // [■ (01,01)] : in used unit, erase 1 times, write 1 times
            // [□ (01,00)] : no used unit, erase 1 times, write 0 times
            printf("\nPage Info, ■ in used, □ no used, (erase times, write times)\n");
            page->info(page, 6);

            // prevent show too many info
            break;
        }

        // prevent show too many info
        break;
    }

    return;
}

#define MY_UNIT_AMT                     (16)
#define MY_PAGE_AMT                     (8)
#define MY_BLOK_AMT                     (8)

int main(int argc, char *argv[])
{

    int blok = MY_BLOK_AMT;
    int page = MY_PAGE_AMT;
    int unit = MY_UNIT_AMT;

    if (argc < 2) {
        printf("Maybe you can use \"8 8 16\"\n");
        printf("Bridge Parameter is : \"1024 64 2048\"\n");
        return 0;
    } else if (argc < 4) {
        jprintf("Invalid args.\n");
        jprintf("usage : %s [block amt] [page amt] [unit amt]\n", argv[0]);
        jprintf("NOTE : unit is 4 byte area.\n");
        jprintf("       ex : [unit amt] = 1 , sizeof(unit) = 4.\n");
        exit(EXIT_FAILURE);
    } else if (!atoi(argv[1])) {
        jprintf("Invalid block amt.\n");
        exit(EXIT_FAILURE);
    } else if (!atoi(argv[2])) {
        jprintf("Invalid page amt.\n");
        exit(EXIT_FAILURE);
    } else if (!atoi(argv[3])) {
        jprintf("Invalid unit amt.\n");
        exit(EXIT_FAILURE);
    } else {
        blok = atoi(argv[1]);
        page = atoi(argv[2]);
        unit = atoi(argv[3]);
    }

    myfs *fs = myfs_new();
    if (!fs) {
        jprintf("myfs new failed\n");
        exit(EXIT_FAILURE);
    } else if (fs->init(fs, blok, page, unit)) {
        jprintf("myfs init failed\n");
        fs->destroy(fs);
        exit(EXIT_FAILURE);
    } else if (fs->mount(fs)) {
        jprintf("myfs mount failed\n");
        fs->destroy(fs);
        exit(EXIT_FAILURE);
    }

    example_vflash_info(fs);
//  example_vflash_block_dump(fs);
//  example_vflash_page_dump(fs);


    printf("\n");

    fs->clear_cnt(fs);
    printf("fs fill log file ... \n");
    example_fs_fill_log_file(fs, 8, 2, 64);
    printf("read count count : %d\n", fs->get_read_cnt(fs));
    printf("prog count count : %d\n", fs->get_prog_cnt(fs));
    printf("erase count count : %d\n", fs->get_erase_cnt(fs));
    printf("sync count count : %d\n", fs->get_sync_cnt(fs));

//  example_vflash_dump_all(fs);
//  example_fs_create_dir(fs, "./joeyoung_dir");
//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file");
//  example_fs_read_file(fs, "./joeyoung_dir/joeyoung_file");

//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file1");
//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file2");
//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file3");
//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file4");
//  example_fs_write_file(fs, "./joeyoung_dir/joeyoung_file5");

//  example_fs_create_dir(fs, "./dir2");
//  example_fs_write_file(fs, "./dir2/file1");
//  example_fs_write_file(fs, "./dir2/file2");
//  example_fs_create_dir(fs, "./dir3");
//  example_fs_write_file(fs, "./dir3/file1");
//  example_fs_write_file(fs, "./dir3/file2");
//  example_fs_create_dir(fs, "./dir4");

    // example_fs_file_list(fs);

    // example_fs_remove_all(fs);

    // example_fs_file_list(fs);
    // example_fs_file_list(fs);

    fs->unmount(fs);

    fs->clear_cnt(fs);
    printf("fs format ... \n");
    example_fs_test_format(fs);
    printf("read count count : %d\n", fs->get_read_cnt(fs));
    printf("prog count count : %d\n", fs->get_prog_cnt(fs));
    printf("erase count count : %d\n", fs->get_erase_cnt(fs));
    printf("sync count count : %d\n", fs->get_sync_cnt(fs));


    fs->mount(fs);

    fs->clear_cnt(fs);
    printf("fs fill log file AGAIN ... \n");
    example_fs_fill_log_file(fs, 8, 2, 64);
    printf("read count count : %d\n", fs->get_read_cnt(fs));
    printf("prog count count : %d\n", fs->get_prog_cnt(fs));
    printf("erase count count : %d\n", fs->get_erase_cnt(fs));
    printf("sync count count : %d\n", fs->get_sync_cnt(fs));

    fs->unmount(fs);

    // fs->save(fs);

    fs->destroy(fs);

    return 0;
}

#endif
