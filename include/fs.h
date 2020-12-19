#include <stdint.h>

#define FS_BLOCKSIZE 1024
#define MAGIC_NUM 0xdec0de
#define MAX_BLOCK_NUM 4096
#define MAX_INODE_NUM 1024
#define MAX_DIR_DEPTH 10


#define TYPE_FILE 0
#define TYPE_DIR 1

typedef struct superblock {
    int32_t magic_num;          // To recognize file system
    int32_t free_block_count;
    int32_t free_inode_count;
    int32_t dir_inode_count;
    uint32_t block_map[128];
    uint32_t inode_map[32];
}Sp_block;


typedef struct inode {
    uint32_t size;
    uint16_t file_type;
    uint16_t link;
    uint32_t block_point[6];
}Inode;


typedef struct dir_item {
    uint32_t inode_id;
    uint16_t valid;
    uint8_t type;
    char name[121];
}Dir_item;

typedef struct inode_buf{
    int block;
    char buf[FS_BLOCKSIZE];
}Inode_buf;


void fs_init();

int fs_create(const char* filename, int flag);

int fs_read(const char* filename, char *buf, int flag);

int fs_write(const char* filename, char * buf, int size);