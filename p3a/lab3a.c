#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext2_fs.h"

int fd;
struct ext2_super_block superblock;

void print_superblock();
void print_group();
void print_bfree();
void print_ifree();
void print_inode();
void print_dirent();
void print_indirect();

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Incorrect number of arguments (%d)\n", argc);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        exit(1);
    }
    // superblock summary
    print_superblock();
    // group summary
    print_group();
    // free block entries
    print_bfree();
    // free I-node entries
    print_ifree();
    // I-node summary
    print_inode();
    // directory entries
    print_dirent();
    // indirect block references
    print_indirect();

    return 0;
}

void print_superblock()
{
    pread(fd, &superblock, sizeof(superblock), 1024);
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
           superblock.s_blocks_count,
           superblock.s_inodes_count,
           EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size,
           superblock.s_inode_size,
           superblock.s_blocks_per_group,
           superblock.s_inodes_per_group,
           superblock.s_first_ino);
}

void print_group()
{
    struct ext2_group_desc group;
    pread(fd, &group, sizeof(group), 1024 + sizeof(superblock));
    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
           0,
           superblock.s_blocks_count,
           superblock.s_inodes_count,
           group.bg_free_blocks_count,
           group.bg_free_inodes_count,
           group.bg_block_bitmap,
           group.bg_inode_bitmap,
           group.bg_inode_table);
}

void print_bfree()
{
    printf("BFREE\n");
}

void print_ifree()
{
    printf("IFREE\n");
}

void print_inode()
{
    printf("INODE\n");
}

void print_dirent()
{
    printf("DIRENT\n");
}

void print_indirect()
{
    printf("INDIRECT\n");
}