/* Meta information about the filesystem */
struct superblock
{
	int num_inodes;
	int num_blocks;
	int size_blocks;
};

struct inode
{
	int size;
	char name[8];
};

struct disk_block
{
	int next_block_num;
	/* Bytes */
	char data[512]
};

/* Initialize the new filesystem */
void create_fs();

/* Load the filesystem */
void mount_fs();

/* Write to the filesystem */
void sync_fs();

/* Debugging and printing */
void print_fs();
