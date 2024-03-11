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

	/* Inodes */
	for (int i = 0; i < sb.num_inodes; i++)
	{
		fread(&(inodes[i]), sizeof(struct inode), 1, file);
	}

	/* Disk blocks */
	for (int i = 0; i < sb.num_blocks; i++)
	{
		fread(&(dbs[i]), sizeof(struct disk_block), 1, file);
	}

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
