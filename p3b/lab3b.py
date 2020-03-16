import collections
import csv
import sys

class superblock:
    def __init__(self, blocks_count, inodes_count, block_size, inode_size, blocks_per_group, \
                 inodes_per_group, first_ino):
        self.blocks_count = blocks_count
        self.inodes_count = inodes_count
        self.block_size = block_size
        self.inode_size = inode_size
        self.blocks_per_group = blocks_per_group
        self.nodes_per_group = inodes_per_group
        self.first_ino = first_ino
    def __str__(self):
        return ','.join(map(str, self.__dict__.values()))

class group:
    def __init__(self, free_blocks_count, free_inodes_count, block_bitmap, inode_bitmap, inode_table):
        self.free_blocks_count = free_blocks_count
        self.free_inodes_count = free_inodes_count
        self.block_bitmap = block_bitmap
        self.inode_bitmap = inode_bitmap
        self.inode_table = inode_table
    def __str__(self):
        return ','.join(map(str, self.__dict__.values()))

class inode:
    def __init__(self, inode_number, file_type, i_mode, i_uid, i_gid, i_links_count, c_time, \
                m_time, a_time, i_size, i_blocks, *args):
        self.inode_number = inode_number
        self.file_type = file_type
        self.i_mode = i_mode
        self.i_uid = i_uid
        self.i_gid = i_gid
        self.i_links_count = i_links_count
        self.c_time = c_time
        self.m_time = m_time
        self.a_time = a_time
        self.i_size = i_size
        self.i_blocks = i_blocks
        self.i_block = args
    def __str__(self):
        return ','.join(map(str, self.__dict__.values()))

class dirent:
    def __init__(self, parent_inode, offset, inode_number, entry_length, name_length, name):
        self.parent_inode = parent_inode
        self.offset = offset
        self.inode_number = inode_number
        self.entry_length = entry_length
        self.name_length = name_length
        self.name = name
    def __str__(self):
        return ','.join(map(str, self.__dict__.values()))

class indirect:
    def __init__(self, inode_number, level, offset, block_number, ref_block_number):
        self.inode_number = inode_number
        self.level = level
        self.offset = offset
        self.block_number = block_number
        self.ref_block_number = ref_block_number
    def __str__(self):
        return ','.join(map(str, self.__dict__.values()))

block_free_set = set()
block_allocated_set = set()
inode_allocated_set = set()
inode_free_set = set()
inode_set = set()
dirent_set = set()
indirect_set = set()
block_dict = collections.defaultdict(int)
inode_dict = collections.defaultdict()

try:
    file_name = sys.argv[1]
except IndexError as error:
    sys.stderr.write("File Name is required...\n")
    sys.exit(1)
finally:
    if len(sys.argv) > 2:
        sys.stderr.write("Invalid arguments...\n")
        sys.exit(1)
try:
    with open(file_name, newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',', )
        for row in reader:
            if row[0] == 'SUPERBLOCK':
                sb = superblock(*list(map(int, row[1:])))
            elif row[0] == 'GROUP':
                g = group(*list(map(int, row[2:7])))
            elif row[0] == 'INODE':
                node = inode(int(row[1]), row[2], int(row[3]), int(row[4]), int(row[5]), int(row[6]), row[7], row[8], row[9],\
                    *list(map(int, row[10:])))
                for b in row[12:]:
                    b = int(b)
                    if b != 0:
                        block_dict[b] = block_dict.get(b, 0) + 1
                inode_set.add(node)
                inode_allocated_set.add(node.inode_number)
                inode_dict[node.inode_number] = node
            elif row[0] == 'BFREE':
                block_free_set.add(int(row[1]))
            elif row[0] == 'IFREE':
                inode_free_set.add(int(row[1]))
            elif row[0] == 'DIRENT':
                dir = dirent(*list(map(int, row[1:6])), row[6])
                dirent_set.add(dir)
            elif row[0] == 'INDIRECT':
                indir = indirect(*list(map(int, row[1:])))
                indirect_set.add(indir)
                block_allocated_set.add(indir.block_number)
                block_allocated_set.add(indir.ref_block_number)
except FileNotFoundError as error:
    sys.stderr.write("File Cannot be Found...\n")
    sys.exit(1)
# print(sb)
# print(g)
# print(inode_free_set)
# print(block_free_set)
reserved_blocks = 4 + sb.inode_size * sb.inodes_count / 1024
# print(reserved_blocks)

for inode in inode_set:
    for i in range(15):
        if inode.i_block[i] in block_free_set:
            sys.stdout.write("ALLOCATED BLOCK {} ON FREELIST\n".format(inode.i_block[i]))
        if inode.i_block[i] > reserved_blocks:
            block_allocated_set.add(inode.i_block[i])
        if i < 12:
            if block_dict[inode.i_block[i]] > 1 and inode.i_block[i] != 0:
                sys.stdout.write("DUPLICATE BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
            if inode.i_block[i] < 0 or inode.i_block[i] > sb.blocks_count:
                sys.stdout.write("INVALID BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
            if 0 < inode.i_block[i] <= reserved_blocks:
                sys.stdout.write("RESERVED BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
        elif i < 13:
            if block_dict[inode.i_block[i]] > 1 and inode.i_block[i] != 0:
                sys.stdout.write("DUPLICATE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
            if inode.i_block[i] < 0 or inode.i_block[i] > sb.blocks_count:
                sys.stdout.write("INVALID INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
            if 0 < inode.i_block[i] <= reserved_blocks:
                sys.stdout.write("RESERVED INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, i))
        elif i < 14:
            if block_dict[inode.i_block[i]] > 1 and inode.i_block[i] != 0:
                sys.stdout.write("DUPLICATE DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 12 + 256))
            if inode.i_block[i] < 0 or inode.i_block[i] > sb.blocks_count:
                sys.stdout.write("INVALID DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 256 + 12))
            if 0 < inode.i_block[i] <= reserved_blocks:
                sys.stdout.write("RESERVED DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 256 + 12))
        else:
            if block_dict[inode.i_block[i]] > 1 and inode.i_block[i] != 0:
                sys.stdout.write("DUPLICATE TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 12 + 256 + 256 * 256))
            if inode.i_block[i] < 0 or inode.i_block[i] > sb.blocks_count:
                sys.stdout.write("INVALID TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 256 * 256 + 256 + 12))
            if 0 < inode.i_block[i] <= reserved_blocks:
                sys.stdout.write("RESERVED TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}\n".format(inode.i_block[i], inode.inode_number, 256 * 256 + 256 + 12))
    if inode.inode_number in inode_free_set:
        sys.stdout.write("ALLOCATED INODE {} ON FREELIST\n".format(inode.inode_number))
referenced_block = block_allocated_set.union(block_free_set)
total_inode = inode_free_set.union(inode_allocated_set)
for b in set(range(8, sb.blocks_count)).difference(referenced_block):
    sys.stdout.write("UNREFERENCED BLOCK {}\n".format(b))
for n in set(range(sb.first_ino, sb.inodes_count)).difference(total_inode):
    sys.stdout.write("UNALLOCATED INODE {} NOT ON FREELIST\n".format(n))

link_dict = collections.defaultdict(int)
for entry in dirent_set:
    parent = entry.parent_inode
    num = entry.inode_number
    if entry.name == "'.'" and num != entry.parent_inode:
        sys.stdout.write("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}\n".format(entry.parent_inode, num, entry.parent_inode))
    if entry.name == "'..'" and num > entry.parent_inode:
        sys.stdout.write("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}\n".format(entry.parent_inode, num, entry.parent_inode))
    if num < 1 or num > sb.inodes_count:
        sys.stdout.write("DIRECTORY INODE {} NAME {} INVALID INODE {}\n".format(entry.parent_inode, entry.name, num))
    elif num in inode_free_set and entry.name != "'..'" and entry.name != "'.'":
        sys.stdout.write("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}\n".format(entry.parent_inode, entry.name, num))
    else:
        link_dict[num] = link_dict[num] + 1
    
for k in range(1, sb.inodes_count):
    if link_dict[k] != (inode_dict[k].i_links_count if inode_dict.get(k) != None else 0):
        sys.stdout.write("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}\n".format(k, link_dict[k], inode_dict[k].i_links_count))
