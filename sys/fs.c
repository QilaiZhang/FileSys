#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

#define BLOCKNUM(inode_num) ((inode_num) / 32 + 1)

Sp_block *sp;
Inode_buf inode_buf;
char buf[FS_BLOCKSIZE];

// void show_sp(Sp_block * sp){
//     printf("%d\n",sp->free_block_count);
//     printf("%d\n",sp->free_inode_count);
// }

void bit_set(uint32_t *data_map, int num)
{
    data_map[num / 32] |= 1 << (num % 32);
}

void bit_reset(uint32_t *data_map, int num)
{
    data_map[num / 32] &= ~(1 << (num % 32));
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

int parse_file_name(char* filename, int *num, char ** dirs){
    char *p = filename;
    while(*p == '/') p++;
    if(*p == '\0'){
        *num = 0;
        return 0;
    }
    int n = 0;
    dirs[n++] = p;
    p = strchr(p, '/');

    while(p){
        if(n >= MAX_DIR_DEPTH){
            printf("Warning: Max dir depth exceed.\n");
            return -1;
        }
        *p = '\0';
        while(*(++p) == '/');
        if(*p == '\0'){
            break;
        }
        dirs[n++] = p;
        p = strchr(p, '/');
    }
    
    *num = n;
    return 0;
}

uint32_t find_dir(int num, char ** dirs){
    uint32_t inode_num = 2;
    int n = 0;

    while(num){
        read_from_disk(BLOCKNUM(inode_num), inode_buf.buf);
        Inode *dir = (Inode *)inode_buf.buf + inode_num % 32;
        if(dir->file_type != TYPE_DIR || dir->link == 0){
            return 0;
        }

        for(int i=0; i < dir->link; i++){
            if(dir->block_point[i] == 0){
                return 0;
            }
            
            read_from_disk(dir->block_point[i], buf);
            Dir_item *d = (Dir_item *)buf;
            int flag = 0;
            for(int i = 0; i < 8; i++){
                if((d+i)->valid == 0){
                    continue;
                }
                if(strcmp(dirs[n], (d+i)->name) == 0){
                    flag = 1;
                    inode_num = (d+i)->inode_id;
                    break;
                }
            }
            
            if(flag == 1){
                break;
            }else if(i == dir->link - 1){
                return 0;
            }
        }
        n++;
        num--;
    }

    return inode_num;
}

int search_free_block(Sp_block *sp){
    uint32_t map;
    for(int i = 1; i < 128; i++){
        map = sp->block_map[i];
        for(int j = 0; j < 8; j++){
            if(!(map & (1 << j))){
                sp->free_block_count--;
                bit_set(sp->block_map, i * 32 + j);
                return i * 32 + j;
            }
        }
    }
    return 0;
}

int search_free_inode(Sp_block *sp, uint32_t *inode_num){
    uint32_t map;
    for(int i = 0; i < 32; i++){
        map = sp->inode_map[i];
        for(int j = 0; j < 8; j++){
            if(!(map & (1 << j))){
                sp->free_inode_count--;
                *inode_num = i * 32 + j;
                bit_set(sp->inode_map, *inode_num);
                return BLOCKNUM(*inode_num);
            }
        }
    }
    return 0;
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
        Inode *root_dir = (Inode *)inode_buf.buf + 2;
        inode_buf.block = 1;
        root_dir->file_type = TYPE_DIR;
        sp->free_block_count -= 1; 
        sp->free_inode_count -= 3;
        sp->dir_inode_count += 1;
        for(int i = 0; i <= 2; i++){
            bit_set(sp->inode_map, i);
        }

        write_to_disk(0, sp);
        write_to_disk(inode_buf.block, inode_buf.buf);
    }

    free(sp);

    if(close_disk() < 0){
        printf("Error: No disk open.\n");
        exit(-1);
    }
    printf("Welcome to Qilai's file system ~\n");
}

int fs_create(const char* filename, int flag){
    char *name = malloc(strlen(filename) + 1);
    char *dirs[MAX_DIR_DEPTH];
    int num;

    strcpy(name, filename);

    if(parse_file_name(name, &num, dirs) < 0){
        return 0;
    }

    if(num == 0){
        printf("Warning: path should include a filename.\n");
        close_disk();
        return 0;
    }

    // open disk
    if(open_disk() < 0){
        printf("Error: Disk already open.\n");
        exit(-1);
    }

    // read super block and find dir
    sp = malloc(FS_BLOCKSIZE);
    read_from_disk(0, sp);

    uint32_t inode_num = find_dir(num-1, dirs);
    if(!inode_num){
        printf("Warning: No such file or directory.\n");
        close_disk();
        return 0;
    }

    // check if block and inode free
    if(sp->free_block_count < 2 || sp->free_inode_count < 2){
        printf("Warning: No free memory.\n");
        close_disk();
        return 0;
    }
    
    // open dir inode
    inode_buf.block = BLOCKNUM(inode_num);
    read_from_disk(inode_buf.block, inode_buf.buf);
    Inode *dir = (Inode *)inode_buf.buf + inode_num % 32;

    // check if dir free
    if(dir->size == 6 * FS_BLOCKSIZE){
        printf("Warning: No free memory.\n");
        close_disk();
        return 0;
    }

    // check if file exist
    for(int i=0; i < dir->link; i++){
        if(dir->block_point[i] == 0){
            break;
        }
        read_from_disk(dir->block_point[i], buf);
        Dir_item *d = (Dir_item *)buf;
        for(int i = 0; i < 8; i++){
            if((d+i)->valid == 0){
                continue;
            }
            if(strcmp(dirs[num-1], (d+i)->name) == 0){
                printf("Warning: file exist.\n");
                close_disk();
                return 0;
            }
        }
    }

    // open free dir block
    int block_num;
    if(dir->link != 0){
        block_num = dir->block_point[dir->link-1];
    }else{
        block_num = 0;
    }

    if((dir->size/sizeof(Dir_item)) % 8 == 0){
        block_num = search_free_block(sp);
        dir->block_point[dir->link] = block_num;
        dir->link++;
    }

    if(!block_num){
        printf("Error: Filesys error in fs_create.\n");
        exit(-1);
    }

    read_from_disk(block_num, buf);
    Dir_item *d = (Dir_item *)buf;
    while(d->valid){
        d++;
        if(d-(Dir_item *)buf == 8){
            // Ensure to memset memeory when removing a file
            printf("Error: Filesys error in fs_create.\n");
        }
    }

    // dir inode write to disk
    dir->size += sizeof(Dir_item);
    write_to_disk(inode_buf.block, inode_buf.buf);

    // fill in dir block data
    d->valid = 1;
    d->type = flag;
    strcpy(d->name, dirs[num-1]);

    // open a new inode
    inode_buf.block = search_free_inode(sp, &inode_num);
    read_from_disk(inode_buf.block, inode_buf.buf);
    dir = (Inode *)inode_buf.buf + inode_num % 32;
    dir->file_type = flag;
    d->inode_id = inode_num;
    
    // write to disk
    write_to_disk(block_num, buf);
    write_to_disk(inode_buf.block, inode_buf.buf);
    write_to_disk(0, sp);

    if(close_disk() < 0){
        printf("Error: No disk open.\n");
        exit(-1);
    }

    free(sp);
    free(name);
    return 0;
}

int fs_read(const char* filename, char * buffer, int flag){
    char *name = malloc(strlen(filename) + 1);
    char *dirs[MAX_DIR_DEPTH];
    int num;

    strcpy(name, filename);

    if(parse_file_name(name, &num, dirs) < 0){
        close_disk();
        return -1;
    }

    // open disk
    if(open_disk() < 0){
        printf("Error: Disk already open.\n");
        exit(-1);
    }

    // read super block and find dir
    sp = malloc(FS_BLOCKSIZE);
    read_from_disk(0, sp);

    uint32_t inode_num;
    inode_num = find_dir(num, dirs);

    if(!inode_num){
        printf("Warning: No such file or directory.\n");
        close_disk();
        return -1;
    }

    // open dir inode
    inode_buf.block = BLOCKNUM(inode_num);
    read_from_disk(inode_buf.block, inode_buf.buf);
    Inode *dir = (Inode *)inode_buf.buf + inode_num % 32;
    int file_num;
    int file_size;

    if(dir->file_type == TYPE_FILE){
        file_num = -2;
        file_size = dir->size;
    }else if(dir->file_type == TYPE_DIR){
        file_size = -2;
        file_num = dir->size / sizeof(Dir_item);
    }

    // open dir block
    for(int i = 0; i < dir->link; i++){
        read_from_disk(dir->block_point[0], buffer + i * FS_BLOCKSIZE);
    }

    if(close_disk() < 0){
        printf("Error: No disk open.\n");
        exit(-1);
    }

    free(sp);
    free(name);

    if(flag == TYPE_DIR)
        return file_num;
    else if(flag == TYPE_FILE)
        return file_size;
}



int fs_write(const char* filename, char * buffer, int size){
    if(size == 0 || size > 6 * FS_BLOCKSIZE){
        return 0;
    }
    
    char *name = malloc(strlen(filename) + 1);
    char *dirs[MAX_DIR_DEPTH];
    int num;

    strcpy(name, filename);

    if(parse_file_name(name, &num, dirs) < 0){
        close_disk();
        return -1;
    }

    // open disk
    if(open_disk() < 0){
        printf("Error: Disk already open.\n");
        exit(-1);
    }

    // read super block and find dir
    sp = malloc(FS_BLOCKSIZE);
    read_from_disk(0, sp);

    uint32_t inode_num;
    inode_num = find_dir(num, dirs);

    if(!inode_num){
        printf("Warning: No such file or directory.\n");
        close_disk();
        return -1;
    }

    // open dir inode
    inode_buf.block = BLOCKNUM(inode_num);
    read_from_disk(inode_buf.block, inode_buf.buf);
    Inode *dir = (Inode *)inode_buf.buf + inode_num % 32;


    char *p = buffer;
    for(int i = 0; i< dir->link; i++){
        write_to_disk(dir->block_point[i], p);
        p += FS_BLOCKSIZE;
        size -= FS_BLOCKSIZE;
    }

    int block_num;
    while(size > 0){
        block_num = search_free_block(sp);
        dir->block_point[dir->link] = block_num;
        dir->link ++;
        write_to_disk(dir->block_point[dir->link], p);
        p += FS_BLOCKSIZE;
        size -= FS_BLOCKSIZE;
    }

    dir->size = size;

    write_to_disk(inode_buf.block, inode_buf.buf);
    write_to_disk(0, sp);

    if(close_disk() < 0){
        printf("Error: No disk open.\n");
        exit(-1);
    }

    free(sp);
    free(name);
    return 0;
}