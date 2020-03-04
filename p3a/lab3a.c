#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext2_fs.h"

int fd, block_size, superblock_offset = 1024;
struct ext2_super_block superblock;

void print_superblock();
void print_group();
void print_bfree(u_int32_t pos);
void print_ifree(u_int32_t pos);
void print_inode(u_int32_t pos);
void print_dirent(u_int32_t pos, u_int32_t parent);
void print_indirect(u_int32_t pos, u_int32_t parent, int level, int level_offset, int local_offset);

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

    // group summary, bfree, ifree, inode, dirent and indirect
    print_group();

    return 0;
}

void print_superblock()
{
    pread(fd, &superblock, sizeof(superblock), superblock_offset);
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
    pread(fd, &group, sizeof(group), superblock_offset + block_size);
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
    print_inode(group.bg_inode_table);
}

void print_bfree(u_int32_t pos)
{
    char c, *buffer;
    u_int32_t i, j;
    buffer = (char *)malloc(sizeof(char) * superblock.s_blocks_count / 8);
    pread(fd, buffer, superblock.s_blocks_count / 8, superblock_offset + (pos - 1) * block_size);
    for (i = 0; i < superblock.s_blocks_count / 8; i++)
    {
        c = buffer[i];
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
    char c, *buffer;
    u_int32_t i, j;
    buffer = (char *)malloc(sizeof(char) * superblock.s_inodes_count / 8);
    pread(fd, buffer, superblock.s_inodes_count / 8, superblock_offset + (pos - 1) * block_size);
    for (i = 0; i < superblock.s_inodes_count / 8; i++)
    {
        c = buffer[i];
        for (j = 0; j < 8; j++)
        {
            if (!(c & 1))
                printf("IFREE,%d\n", i * 8 + j + 1);
            c >>= 1;
        }
    }
}

void print_inode(u_int32_t pos)
{
    struct ext2_inode inode;
    char file_type, ctime_str[32], mtime_str[32], atime_str[32];
    time_t ctime, mtime, atime;
    u_int32_t i, j;
    for (i = 0; i < superblock.s_inodes_count; i++)
    {
        pread(fd, &inode, superblock.s_inode_size, superblock_offset + (pos - 1) * block_size + i * superblock.s_inode_size);
        if (inode.i_mode && inode.i_links_count)
        {
            // get file type
            file_type = '?';
            if (S_ISREG(inode.i_mode))
                file_type = 'f';
            else if (S_ISDIR(inode.i_mode))
                file_type = 'd';
            else if (S_ISLNK(inode.i_mode))
                file_type = 's';

            // process time
            ctime = inode.i_ctime;
            mtime = inode.i_mtime;
            atime = inode.i_atime;
            strftime(ctime_str, 32, "%x %X", gmtime(&ctime));
            strftime(mtime_str, 32, "%x %X", gmtime(&mtime));
            strftime(atime_str, 32, "%x %X", gmtime(&atime));

            // print inode summary
            printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
                   i + 1,
                   file_type,
                   inode.i_mode & 0xFFF,
                   inode.i_uid,
                   inode.i_gid,
                   inode.i_links_count,
                   ctime_str,
                   mtime_str,
                   atime_str,
                   inode.i_size,
                   inode.i_blocks);
            if (file_type == 'f' || file_type == 'd' || (file_type == 's' && inode.i_size >= 60))
                for (j = 0; j < EXT2_N_BLOCKS; j++)
                    printf(",%d", inode.i_block[j]);
            printf("\n");

            // print directory entries
            if (file_type == 'd')
                for (j = 0; j < EXT2_NDIR_BLOCKS; j++)
                    if (inode.i_block[j])
                        print_dirent(inode.i_block[j], i + 1);

            // print indirect block references
            if (file_type == 'f' || file_type == 'd')
                for (j = EXT2_IND_BLOCK; j <= EXT2_TIND_BLOCK; j++)
                    if (inode.i_block[j])
                        print_indirect(inode.i_block[j], i + 1, j - EXT2_IND_BLOCK, EXT2_IND_BLOCK, 0);
        }
    }
}

void print_dirent(u_int32_t pos, u_int32_t parent)
{
    int i, offset = 0;
    struct ext2_dir_entry dir;
    for (i = 0; i < block_size; i += offset)
    {
        pread(fd, &dir, sizeof(dir), superblock_offset + (pos - 1) * block_size + i);
        if (dir.inode)
            printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n",
                   parent,
                   i,
                   dir.inode,
                   dir.rec_len,
                   dir.name_len,
                   dir.name);
        offset = dir.rec_len;
    }
}

void print_indirect(u_int32_t pos, u_int32_t parent, int level, int level_offset, int local_offset)
{
    if (level < 0)
        return;
    int i, offset;
    u_int32_t buffer;
    for (i = 0; i < block_size / 4; i++)
    {
        offset = superblock_offset + (pos - 1) * block_size + i * sizeof(buffer);
        if (level >= 1 && level_offset == EXT2_IND_BLOCK)
            level_offset += 256;
        if (level == 2)
            level_offset += 256 * 256;

        pread(fd, &buffer, sizeof(buffer), offset);
        if (buffer)
        {
            local_offset += i;
            printf("INDIRECT,%d,%d,%d,%d,%d\n",
                   parent,
                   level + 1,
                   level_offset + local_offset,
                   pos,
                   buffer);
            print_indirect(buffer, parent, level - 1, level_offset, local_offset);
        }
    }
}