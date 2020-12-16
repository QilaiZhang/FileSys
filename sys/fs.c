#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

Sp_block *sp;
Inode_buf inode_buf;
Inode *cur_dir;

void bit_set(uint32_t *data_map, int num)
{
    data_map[num / 32] |= 1 << num;
}

void bit_reset(uint32_t *data_map, int num)
{
    data_map[num / 32] &= ~(1 << num);
}

void read_from_disk(unsigned int block_num, void * p)
{
    if(disk_read_block(block_num * 2, (char *)p) < 0){
        printf("Error: Read from disk error.\n");
        exit(-1);
    }
    if(disk_read_block(block_num * 2 + 1, (char *)p + DEVICE_BLOCK_SIZE) < 0){
        printf("Error: Read from disk error.\n");
        exit(-1);
    }
}

void write_to_disk(unsigned int block_num, void * p){
    if(disk_write_block(block_num * 2, (char *)p) < 0){
        printf("Error: Write to disk error.\n");
        exit(-1);
    }
    if(disk_write_block(block_num * 2 + 1, (char *)p + DEVICE_BLOCK_SIZE) < 0){
        printf("Error: Write to disk error.\n");
        exit(-1);
    }
}

void fs_init()
{
    // open disk
    if(open_disk() < 0){
        printf("Error: Disk already open.\n");
        exit(-1);
    }

    // read super block
    sp = malloc(FS_BLOCKSIZE);
    read_from_disk(0, sp);
    if(sp->magic_num != MAGIC_NUM){
        if(sp->magic_num != 0){
            printf("Disk has been damaged.\n");
            memset(sp, 0, FS_BLOCKSIZE);
        }
        printf("A new disk has been created.\n");

        // init super block
        sp->magic_num = MAGIC_NUM;
        sp->free_block_count = MAX_BLOCK_NUM - 1;
        sp->free_inode_count  = MAX_INODE_NUM;
        for(int i = 0; i <= MAX_INODE_NUM * sizeof(Inode)/ FS_BLOCKSIZE; i++){
            bit_set(sp->block_map, i);
        }

        // init root dir
        memset(inode_buf.buf, 0, FS_BLOCKSIZE);
        Inode *root_dir = (Inode *)inode_buf.buf;
        inode_buf.block = 1;
        root_dir->file_type = TYPE_DIR;
        sp->free_inode_count -= 1;
        sp->dir_inode_count += 1;
        for(int i = 0; i <= 2; i++){
            bit_set(sp->inode_map, i);
        }

        write_to_disk(0, sp);
        write_to_disk(inode_buf.block, inode_buf.buf);
        cur_dir = root_dir;
    }
    
    if(!cur_dir){
        inode_buf.block = 1;
        read_from_disk(inode_buf.block, inode_buf.buf);
        cur_dir = (Inode *)inode_buf.buf;
    }

    if(close_disk() < 0){
        printf("Error: No disk open.\n");
        exit(-1);
    }
    printf("Welcome to Qilai's file system ~\n");
}

Mfile* fs_open(const char* filename, int flag)
{
    
    return NULL;
}

int fs_close(Mfile* fd)
{
    return 0;
}

int fs_read(Mfile* fd, void* buf, int n){

    return 0;
}

int fs_write(Mfile* fd, const void* buf, int n){
    return 0;
}

int fs_mkdir(const char* dirname){
    return 0;
}
