#include <stdint.h>

struct superblock {
    int32_t magic_num;          // To recognize file system
    int32_t free_block_count;
    int32_t free_inode_count;
    int32_t dir_inode_count;
    uint32_t block_map[128];
    uint32_t inode_map[32];
};


struct inode {
    uint32_t size;
    uint16_t file_type;
    uint16_t link;
    uint32_t block_point[6];
};


struct dir_item {
    uint32_t inode_id;
    uint16_t valid;
    uint8_t type;
    char name[121];
};
