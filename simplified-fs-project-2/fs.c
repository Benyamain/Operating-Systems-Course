#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"

/*
struct superblock
{
	int num_inodes;
	int num_blocks;
	int size_blocks;
};

struct inode
{
	int size;
	int first_block;
	char name[8];
};

struct disk_block
{
	int next_block_num;
	char data[512]
};
*/

struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;

/* Initialize the new filesystem */
void
create_fs()
{
	sb.num_inodes = 10;
	sb.num_blocks = 100;
	sb.size_blocks = sizeof(struct disk_block);

	inodes = malloc(sizeof(struct inode) * sb.num_inodes);
	for (int i = 0; i < sb.num_inodes; i++)
	{
		/* Not allocated */
		inodes[i].size = -1;
		inodes[i].first_block = -1;
		strcpy(inodes[i].name, "Empty!!");
	}

	dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);
	for (int i = 0; i < sb.num_blocks; i++)
	{
		/* Not allocated */
		dbs[i].next_block_num = -1;
		/* No need to initialize the data member */
	}
}

/* Load (read) data from the filesystem */
void
mount_fs()
{
	FILE *file;
	file = fopen("fs_data", "r");

	/* Superblock */
	fread(&sb, sizeof(struct superblock), 1, file);

	inodes = malloc(sizeof(struct inode) * sb.num_inodes);
	dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);

	/* Another more efficient way to achieve the same functionality as code logic to sync */
	fread(inodes, sizeof(struct inode), sb.num_inodes, file);
	fread(dbs, sizeof(struct disk_block), sb.num_blocks, file);

	fclose(file);
}

/* Write to the filesystem */
void
sync_fs()
{
	FILE *file;
	file = fopen("fs_data", "w+");

	/* Superblock */
	fwrite(&sb, sizeof(struct superblock), 1, file);

	/* Inodes */
	for (int i = 0; i < sb.num_inodes; i++)
	{
		fwrite(&(inodes[i]), sizeof(struct inode), 1, file);
	}

	/* Disk blocks */
	for (int i = 0; i < sb.num_blocks; i++)
	{
		fwrite(&(dbs[i]), sizeof(struct disk_block), 1, file);
	}

	fclose(file);
}

void
print_fs()
{
	/* Superblock printing */
	printf("Superblock data ...\n");
	printf("\tsb.num_inodes: %d\n", sb.num_inodes);
	printf("\tsb.num_blocks: %d\n", sb.num_blocks);
	printf("\tsb.size_blocks: %d\n", sb.size_blocks);

	/* Inode printing */
	for (int i = 0; i < sb.num_inodes; i++)
	{
		printf("\tinodes[i].size: %d, inodes[i].first_block: %d, inodes[i].name: %s\n", inodes[i].size,  inodes[i].first_block, inodes[i].name);
	}

	for (int i = 0; i < sb.num_blocks; i++)
	{
		printf("\tBlock Number: %d, dbs[i].next_block_num: %d\n", i, dbs[i].next_block_num);
	}
}
