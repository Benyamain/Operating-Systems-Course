#include "posix-calls.h"
#include "Llibc.h"
#include "Lcli.h"
#include "walkfunctions.h"

struct buf* bread(uint, uint);
void brelse(struct buf*);
void Lprintf(const char *format, ...);

extern struct superblock SB;
extern int DEVFD;
extern struct {
  uint inum;
  char name[MAXPATH];
} CWD;

/*
 File walkfunctions.c

  Functions are described below.  Some functions here will need to
  walk through all the blocks of a file, given its inode.
*/


int
getinode(struct dinode *inode, uint inodenum)
{
/*
  Find the inode struct for the given inode number, and fill in the
  caller supplied pointer to struct dinode.
  Return 0 on success, -1 on error.

  Used by several other functions!
*/

	//Using the inode number given to us + the Mailman algorithm to find out the block number
	//where that inode is stored
	uint blockno = SB.inodestart + inodenum / (BSIZE / sizeof(struct dinode));

    struct buf *b;

	// Reads data from the block that the inode is stored [in the inode table]
	b = bread(DEVFD, blockno);

	// Extract an inode from that section
	uint inode_size = sizeof(struct dinode);
	uint inodes_per_block = BSIZE/inode_size;

	struct dinode *retrieved_inode =  (struct dinode *) &b->data[(inode_size * (inodenum % inodes_per_block))];

	//Error handling for when the inode the caller wants to retrieve is 0 (a free inode)
	if (retrieved_inode->type == 0) {
		return -1;
	}

	// Copy that inode to the parameter passed
	Lmemcpy(inode, retrieved_inode, inode_size);

	//Release the buffer
	brelse(b);

	return 0;
}

uint
find_name_in_dirblock(uint blockptr, const char *nam)
{
/*
  Assuming that blockptr points to a block of some directory file,
  find a directory entry with matching name and return its inode number.
  If no match, return 0.

  Calller must ensure that blockptr points to a block of a directory file.

  Used by find_dent() below.
*/
	struct buf *b;
	char dname[100];

	Lstrcpy(dname, nam);

	//Reads the directory file (will contain a list of directory entries)
	b = bread(DEVFD, blockptr);

	struct dirent* de;

	//Loops through the size of the directory entry block for each directory entry that shows up in the block
	for(int i = 0; i < (BSIZE / sizeof(struct dirent)); i++){

		//Assigns de to an entry
		de = (struct dirent*)(b->data + i * sizeof(struct dirent));

		//Compares the name the user wants to search
		if(!Lstrcmp(de->name, dname))
			return de->inum;
	}

	return 0;
}

uint
find_dent(uint inum, const char *name)
{
/*
  For the given inode number and the given name:
  If this inode is a directory file and there is a directory entry in it
  whose name matches the given name, return the inode number of that
  matching entry; else return 0

  Used by function namei() below.
*/

	char dname[100];
	uint dir_inum;

	Lstrcpy(dname, name);

	struct dinode inode = {};

	//Gets the inode data, if the inode is not found
	//err returns -1, so we handle for that
	int err = getinode(&inode, inum);

	if (err == -1) {
		return 0;
	}

	//Check if inode is of type directory
	if (inode.type != 1) {
		return 0;
	}

	//If that inode is a directory, then it's data block will hold all the
	//directory entries, so we find whether the name the user is searching for exists
	for (int i = 0; inode.addrs[i] != 0; i++) {
		dir_inum = find_name_in_dirblock(inode.addrs[i], dname);
	}

	if (dir_inum == 0) {
		return 0;
	}

	return dir_inum;
}

uint namei(const char *pathname)
{
/*
  Convert pathname to inode number (as in class/quiz).

  A fundamental filesystem algorithm!
*/

	char fn[50];
	char *tokstart, *tokend;

	Lstrcpy(fn, pathname);

	uint inum = 0;

	if(fn[0]== '/'){
		inum = ROOTINO;
	}else{
		inum = namei(CWD.name);
	}

	// Tokenize the pathname
	tokstart = fn;
	tokend = Lstrchr(fn, '/');

	while (tokstart != NULL)
	{
		if (tokend != NULL)
		{
			// Terminate the end
			*tokend = '\0';
		}

		if (Lstrcmp(tokstart, "") != 0)
		{
			inum = find_dent(inum, tokstart);

			if (inum == 0)
			{
				return 0;
			}
		}

		if (tokend != NULL)
		{
			// Restore
			*tokend = '/';
			tokstart = tokend + 1;
			tokend = Lstrchr(tokstart, '/');
		}
		else
		{
			// No tokens
			tokstart = NULL;
		}
	}

	return inum;
}

void lsdir(uint blockptr)
{
/*
  Print the list of directory entries in the block pointed to by blockptr.
  Caller must ensure that blockptr points to a block of a dirctory file.

  Easy function, but make sure to begin with bread() and end with brelse().

  Used by lspath().
*/
	// start with bread() to read in block contents
	struct buf* buffer = bread(DEVFD, blockptr);
	struct dinode inode;
	struct dirent* de;

	// Loop over dirent in bloc based on how many dirent can fit there
	for (int d = 0; d < (BSIZE / sizeof(struct dirent)); d++)
	{
		// Read from dirent
		de = (struct dirent*)(buffer->data + d * sizeof(struct dirent));

		// Account for nonzero
		if (de->inum != 0)
		{
			getinode(&inode, de->inum);
			Lprintf("%d %20s %10d  %d %d\n", inode.type, de->name, inode.nlink, de->inum, inode.size);
		}
	}

	// Release the buffer
	brelse(buffer);
}

int lspath(const char *pathname)
{
/*
  If pathname is a directory, list its contents.

  Use namei, getinode, and lsdir().
*/

	char fn[50];

	Lstrcpy(fn, pathname);

	uint inum = namei(fn);

	struct dinode inode = {};
	getinode(&inode, inum);

	// Check if inode is not a dir
	if (inode.type != 1) {
		return -1;
	}

	for (int i = 0; inode.addrs[i] != 0; i++) {
		lsdir(inode.addrs[i]);
	}

	return 0;
}
