#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext2_fs.h"

int fd, block_size;
struct ext2_super_block superblock;

void print_superblock();
void print_group();
void print_bfree(u_int32_t pos);
void print_ifree(u_int32_t pos);
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
    // print_bfree();
    // free I-node entries
    // print_ifree();
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
    block_size = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
           superblock.s_blocks_count,
           superblock.s_inodes_count,
           block_size,
           superblock.s_inode_size,
           superblock.s_blocks_per_group,
           superblock.s_inodes_per_group,
           superblock.s_first_ino);
}

void print_group()
{
    struct ext2_group_desc group;
    pread(fd, &group, sizeof(group), 2 * block_size);
    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
           0,
           superblock.s_blocks_count,
           superblock.s_inodes_count,
           group.bg_free_blocks_count,
           group.bg_free_inodes_count,
           group.bg_block_bitmap,
           group.bg_inode_bitmap,
           group.bg_inode_table);
    print_bfree(group.bg_block_bitmap);
    print_ifree(group.bg_inode_bitmap);
}

void print_bfree(u_int32_t pos)
{
    char c, buffer[8];
    u_int32_t i, j;
    pread(fd, &buffer, superblock.s_blocks_count / 8, pos * block_size);

    for (i = 0; i < superblock.s_blocks_count / 8; i++)
    {
        c = buffer[i];
        //printf("byte: %d\n", c);
        for (j = 0; j < 8; j++)
        {
            if (!(c & 1))
                printf("BFREE,%d\n", i * 8 + j + 1);
            c >>= 1;
        }
    }
}

void print_ifree(u_int32_t pos)
{
    char c, buffer[8];
    u_int32_t i, j;
    pread(fd, &buffer, superblock.s_inodes_count / 8, pos * block_size);
    for (i = 0; i < superblock.s_inodes_count / 8; i++)
    {
        c = buffer[i];
        //printf("byte: %d\n", c);
        for (j = 0; j < 8; j++)
        {
            if (!(c & 1))
                printf("IFREE,%d\n", i * 8 + j + 1);
            c >>= 1;
        }
    }
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