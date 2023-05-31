/* file system format on disk and struct & operation in kernel-OS */
#ifndef _FS_H
#define _FS_H

#include <type.h>

#define BLOCK_SIZE 4096     //4KB
#define MAX_FILE_LENGTH 10

#define SECTOR_SIZE 512
#define BLOCK_SIZE 4096

#define FS_START_ADDR  (1 << 29)
#define FS_SIZE  (1 << 21)
#define SUPERBLOCK_SIZE (1)
#define SECTORMAP_ADDR  (1 << 29 + 1 << 9) 
#define SECTORMAP_SIZE  (1 << 9)
#define INODEMAP_ADDR  (1 << 29 + 1 << 9 + 1 << 18)
#define INODEMAP_SIZE  (1)
#define INODEBLOCK_ADDR  (1 << 29 + 1 << 9 + 1 << 18 + 1 << 9)
#define INODEBLOCK_SIZE  (1 << 9)
#define DATABLOCK_ADDR  (1 << 29 + 1 << 9 + 1 << 18 + 1 << 9 + 1 << 18)
#define DATABLOCK_SIZE  (1 << 21)

#define DIRECT 3584 //7*512
#define INDIRECT 200192 //7*512+3*128*512(200K)
#define D_INDIRECT 16977408 //7*512+3*128*512+2*128*128*512(16M)
#define T_INDIRECT 1090719232 //7*512+3*128*512+2*128*128*512+128*128*128*512(1G+)
#define BLOCK_NUM 128

#define FIRST_SECTOR  (1 << 20)
#define SECTORMAP_SECTOR_OFFSET  (1)
#define INODEMAP_SECTOR_OFFSET  (513)
#define INODEBLOCK_SECTOR_OFFSET  (514)
#define DATABLOCK_SECTOR_OFFSET  (1026)

#define DIRECT_NUM 7
#define INDIRECT_NUM 3
#define D_INDIRECT_NUM 2
#define DENTRY_NUM 15

#define FILE 0
#define DIR 1

#define MAGIC 0x66666666
#define DENTRY_SIZE sizeof(dentry_t)
#define INODE_SIZE sizeof(inode_t)
#define MKDIR 0
#define NOT_MKDIR 1
#define VALID 1
#define INVALID 0

#define O_RDONLY 1 /* read only open */
#define O_WRONLY 2 /* write only open */
#define O_RDWR 3 /* read/write open */

#define MAX_FILE 100

typedef struct superblock {
    uint32_t magic_number;

    uint32_t fs_size;
    uint32_t fs_start;

    uint32_t sectormap_size;
    uint32_t sectormap_start;

    uint32_t inodemap_size;
    uint32_t inodemap_start;

    uint32_t inode_size;
    uint32_t inode_start;

    uint32_t datablock_size;
    uint32_t datablock_start;
}superblock_t;

typedef struct dentry {
    char name[MAX_FILE_LENGTH];
    uint32_t mode;       // FILE/DIR
    uint32_t ino;        // inode number of file/dir in this dir
    uint32_t valid;
}dentry_t;

typedef struct inode {
    uint32_t ino;
    uint8_t  mode;                  // Inode mode: FILE/DIR
    uint8_t  type;                  // File mode: R/W/RW
    uint16_t link;
    uint32_t size;                  // Size of file (bytes)
    uint32_t valid_size;            // used data/dentry bytes
    uint32_t modified_time;
    uint32_t direct_block[DIRECT_NUM];
    uint32_t indirect_block[INDIRECT_NUM];
    uint32_t d_indirect_block[D_INDIRECT_NUM];
    uint32_t t_indirect_block;
    dentry_t identry[DENTRY_NUM];
}inode_t;

typedef struct file
{
    uint8_t inode;
    uint8_t type;
    uint32_t read_point;
    uint32_t write_point;
}file_t;

int do_mkfs(void);
int do_statfs(void);
void do_fs_ls(void);
void do_mkdir(char* argv);
void do_cd(char* argv);
void do_rmdir(char* argv);
void do_touch(char* argv);
int do_fopen(char* argv, int mode);
int do_fread(int fd, char *buff, int size);
int do_fwrite(int fd, char *buff, int size);
void do_close(int fd);
void do_cat(char* argv);
int do_fs_ls_l(void);
void do_ln(char* argv1, char* argv2);
void do_rm(char* argv);

inode_t* get_inode(int location);
inode_t* relocation(inode_t* inode, char* argv, int mode);
void change_inode_bitmap(int location, int change_number);
void change_sector_bitmap(int location, int change_number);
void write_inode(inode_t* inode, int location);
int get_file_number(void);
int get_inode_number(void);
int get_datablock_number(void);
uint8_t* get_datablock(int location);
void write_datablock(uint8_t* datablock, int location);

int do_lseek(int fd,int offset,int whence);


int user_location;
file_t file[MAX_FILE];

#endif