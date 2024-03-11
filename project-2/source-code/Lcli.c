#include "Lcli.h"
#include "Llibc.h"
#include "posix-calls.h"
#include "walkfunctions.h"

#define LINESIZE 1024 /* Line buffer size */
#define NTOKS 128     /* Max number of tokens in a line */
#define STACKSIZE 100

/* From Lbio.c */
void binit(void);
struct buf *bread(uint, uint);
void brelse(struct buf *);
void bsync();

/*
 Since we are only going to use a single device, and since its superblock
will not change, we can have a global int DEVFD after opening fs.img,
and a global struct suberblock SB!  Also, since throughout the duration
of the CLI, we will have to track CWD, we will have a global struct for it;
*/

/* Our globals */
int DEVFD;
struct superblock SB;
struct {
  uint inum;
  char name[MAXPATH];
} CWD;

void 
devfd_init(char *devpath) {
  /* First do this readonly to implement pwd, ls, cd, and upload commands */
  if ((DEVFD = Lopen(devpath, O_RDWR)) < 0) {
    Lfprintf(2, "Could not open %s\n", devpath);
    Lexit(2);
  }
}

void 
superblock_init(int devfd) {
  struct buf *b;

  if ((b = bread(DEVFD, 1)) == 0) {
    Lfprintf(2, "Could not read superblock\n");
    Lexit(3);
  }

  Lmemcpy(&SB, &b->data[0], sizeof(struct superblock));
  brelse(b);
}

void 
cwd_init(void) {
  CWD.inum = 1;
  Lstrcpy(CWD.name, "/");
}

//Tokenizes the user input
int 
tokenize_input(char *buf, char *tokens[]) {

  for (int i = 0; i < 100; i++) {
    tokens[i] = NULL;
  }

  char *start = buf;
  int num_tokens = 0;

  while (*start != '\0') {

    // Skips all the leading whitespace
    while (Lisspace(*start)) {
      start++;
    }

    // Makes sure we aren't at the end of the string
    if (*start == '\0') {
      break;
    }

    // Looks for the end of the current token/word/command
    char *end = start;
    while (!Lisspace(*end) && *end != '\0') {
      end++;
    }

    *end = '\0';

    // Increase the number of tokens and adds it to the tokens array
    tokens[num_tokens++] = start;

    // Start again for the next token
    start = end + 1;
  }

  tokens[num_tokens] = NULL;

  return num_tokens;
}

//Works with concatenation
void 
Lstrconcat(char *str1, char *str2, char result[500]) {
  int i =0;

  for (char *c = str1; *c != '\0'; c++) {
    result[i] = *c;
    i++;
  }

  //If the first string isnt root and the second string doesnt start with /
  //so str1: , str2: /, doesnt give //
  if (str2[0] != '/' && (Lstrcmp(str1, "/") != 0)){
    result[i++] = '/';
  }

  //starting from where result[i] ended and adding str2 to it
  for (char *c = str2; *c != '\0'; c++) {
    result[i] = *c;
    i++;
  }

  result[i] = '\0';
}

void 
recursive_ls(char *pathname, char *nextDir) {
  struct dinode inode;
  struct dinode next_inode;
  char concatPath[500];

  Lstrconcat(pathname, nextDir, concatPath);

  // Getting the inode structure for the specified path
  uint inode_num = namei(concatPath);
  getinode(&inode, inode_num);

  // Reading block contents (so we can get the type of each entry)
  struct buf *buffer = bread(DEVFD, inode.addrs[0]);
  struct dirent *de;

  //Prints the directory that is being looped over
  Lprintf("\n%s: \n", concatPath);
  lspath(concatPath);

  // Loop over dirent in block based on how many dirent can fit there
  for (int d = 0; d < (BSIZE / sizeof(struct dirent)); d++) {
    // Read from dirent
    de = (struct dirent *)(buffer->data + d * sizeof(struct dirent));

    // Inode of the data found in the block
    int err = getinode(&next_inode, de->inum);

    // Only move on if the entry is a directory (==1) and not "." or ".."
    if (err != -1 && next_inode.type == 1 && Lstrcmp(de->name, ".") != 0 &&
        Lstrcmp(de->name, "..") != 0) {
		recursive_ls(concatPath, de->name);
    }
  }

  brelse(buffer);

  Lmemset(concatPath, '\0', sizeof(concatPath));

}

void
Lstrrev(char *str)
{
	int length = Lstrlen(str);
	int start = 0;
	int end = length - 1;

	while(start < end){
		//Swap characters at start and end
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;

		start++;
		end--;
	}
}

void
createfunc
(char *path, int create_new_inode){
    // Tokenize the path
    char path_copy[100];
    Lstrcpy(path_copy, path);

    if (Lstrchr(path_copy, '/') != NULL) {
       Lprintf("A file cannot have the character '/' in it\n");
       return;
    }

	struct dinode parent_inode = {};
    getinode(&parent_inode, CWD.inum);

	uint found_path = 0;

	for(int i = 0; parent_inode.addrs[i] != 0; i++) {
		found_path = find_name_in_dirblock(parent_inode.addrs[i], path);
	}

	//A file with the name already exists
	if (found_path != 0) {
		Lprintf("A file/directory with the same name already exists\n");
		return;
	}

    // Find an empty inode
	struct dinode new_inode = {};
	uint new_inum;

    if (create_new_inode == 0) {
       for(int k = 1; k < (BSIZE / sizeof(struct dirent)); k++){
           int valid = getinode(&new_inode, k);
           if(valid == -1) {
              new_inum = k;
              break;
           }
       }
	} else {
		new_inum = create_new_inode;
	}

    if (new_inum == 0) {
        // No empty inode found
        Lprintf("Error: No empty inode available.\n");
        return;
    }

    // Write the new inode entry for a regular file
    // Note: we aren't sure how big the new inode should be
    new_inode.type = T_FILE;
    new_inode.nlink = 1;
    new_inode.size = 1024;

    // Read the inode in the inode table
    struct buf *b = bread(DEVFD, IBLOCK(new_inum, SB));

	uint inodesize = sizeof(struct dinode);
	uint inodes_per_block = BSIZE / inodesize;

	b->dirty = 1;

	//Setting the inode in the buffer cache
    Lmemset(&b->data[(new_inum % inodes_per_block) * sizeof(struct dinode)], 0, sizeof(struct dinode));
    Lmemcpy(&b->data[(new_inum % inodes_per_block) * sizeof(struct dinode)], (const char*)&new_inode, sizeof(struct dinode));

    brelse(b);

    //Working on the parent inode
    struct dinode pinode = {};
    getinode(&pinode, CWD.inum);

    //Reads the parents directory file and add the directory entry for the 
    //newly allocated inode
    struct dirent* de;
    struct buf* buffer = bread(DEVFD, pinode.addrs[0]);
    int d;

    for (d = 0; d < (BSIZE / sizeof(struct dirent)); d++){
    	//Read from dirent
        de = (struct dirent *)(buffer->data + d * sizeof(struct dirent));

        //Find a free dirent
	    if(de->inum == 0){
          break;
    	}
	}

    //Define the inode
	struct dirent new_de = {};
    Lstrcpy(new_de.name, path);
    new_de.inum = new_inum;

    //Adds the directory entry to the buffer cache
    Lmemset(&buffer->data[sizeof(struct dirent) * (d)], 0, sizeof(struct dirent));
    Lmemcpy(&buffer->data[sizeof(struct dirent) * (d)], (const char*)&new_de, sizeof(struct dirent));

    buffer->dirty = 1;
    brelse(buffer);
}


// We unfortunately ran out of time, because I took a wrong turn while doing upload and it took me a while.
// However, this would probably be easiest done to putting ls -R in a file and calling upload path.
void
uploadTree(char *path)
{
	// TODO: Dump current ls -R / output text to host
	// TODO: uploadtree filename
	// TODO: Dump output of exec ls -R to specified txt filename
	// TODO: Filename should be dumped on riscv host
	// TODO: Currently on the MIT xv6 filesystem

	// TODO: Creat to make a txt file
	// TODO: Write to that file the output of ls -R
	// TODO: Save it to MIT xv6 filesystem
	// TODO: Transferring internal file to host machine, but how to achieve this?
}

void
upload(char *path, char *filename)
{
	uint pathInum = namei(path);
    	struct dinode inode = {};
    	int fd;

    	fd = Lopen(filename, O_CREAT | O_RDWR);
    	getinode (&inode, pathInum);

    	for (int i = 0; inode.addrs[i] != 0; i++){
        	struct buf* buffer = bread(DEVFD, inode.addrs[i]);

        	Lwrite(fd, buffer->data, 1024);

        	brelse(buffer);
    	}

    	Lclose(fd);	
}

int
Lmain(int argc, char *argv[])
{
  if (argc < 2) {
    Lprintf("Usage:  %s fs_img_path\n", argv[0]);
    return 1;
  }

  devfd_init(argv[1]);

  binit();

  superblock_init(DEVFD);

  cwd_init();

  /* Analyze the superblock */

  uint inodesize = sizeof(struct dinode);

  /* Directories */

  char buf[LINESIZE];

  Lprintf("fscli> ");

  // Looping through buffer
  while (Lread(0, buf, LINESIZE) > 0) {
    int linelen;
    for (linelen = 0; buf[linelen] != '\n' && linelen < LINESIZE; linelen++);
    buf[linelen] = 0;

    /* Analyze line typed by user ... */
    if (Lstrcmp(buf, "") == 0) {
      Lprintf("fscli> ");
      for (int i = 0; i < LINESIZE; i++) {
        buf[i] = '\0';
      }
      continue;
    }

    // Tokenize inputs
    char *tokens[100];
    int num_of_tokens = tokenize_input(buf, tokens);

    /*
     ------------------
     --- HELP COMMAND---
     ------------------
    */

    if (Lstrcmp(tokens[0], "help") == 0 && num_of_tokens == 1) {
      Lprintf("Command\t\t\tDescription\n");
      Lprintf("----------------------------------------------------------------"
              "-----------\n");
      Lprintf("help\t\t\tPrint this table\n");
      Lprintf("pwd\t\t\t\tShow current directory (path and inode)\n");
      Lprintf("cd [path]\t\tChange directory to path (or to /)\n");
      Lprintf("ls [-d] [-R] [path]\tList path as in ls -ail\n");
      Lprintf("creat path\t\tCreate file at path (like touch)\n");
      Lprintf("mkdir path\t\tCreate directory at path\n");
      Lprintf("unlink path\t\tLike rm and rmdir\n");
      Lprintf("link oldpath newpath\tLike ln\n");
      Lprintf("sync\t\t\tWrite all cached “dirty” buffers to device blocks\n");
      Lprintf("quit\t\t\tExit CLI (should also sync)\n");
      Lprintf(
          "uploadtree filename\tDump current ls -R / output text to host\n");
      Lprintf("upload path filename\tCopy path in filesystem image to host\n");
      Lprintf("download filename path\tCopy a host file into filesystem image "
              "at path\n");
      Lprintf("----------------------------------------------------------------"
              "-----------\n");
    } else if (Lstrcmp(tokens[0], "help") == 0 && num_of_tokens != 1) {
      Lprintf("ERROR: Incorrect Usage of 'help' command\n");
    }

    /*
     ------------------
     --- PWD COMMAND---
     ------------------
    */

    if (Lstrcmp(tokens[0], "pwd") == 0) {
      if (num_of_tokens != 1) {
        Lprintf("Incorrect usage of command 'pwd'. Command 'pwd' does not have "
                "any input parameters.\n");
      } else {
        Lprintf("%s\n", CWD.name);
     }
    }

    /*
     ------------------
     --- CD COMMAND---
     ------------------
    */

    if (Lstrcmp(tokens[0], "cd") == 0) {

      // Handling cd or 'cd /'
      if (num_of_tokens == 1 || (num_of_tokens == 2 && !Lstrcmp(tokens[1], "/")) ) {
        Lstrcpy(CWD.name, "/");

        CWD.inum = 1;
        Lprintf("fscli> ");

	//Clears the buffer
	for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }
        continue;
      }

      // Error handling
      if (num_of_tokens != 2) {
        Lprintf("cd accepts only one argument\n");
        Lprintf("fscli> ");

        for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }

        continue;
      }

      if(tokens[1][0] == '/'){
        CWD.inum = 1;
        Lstrcpy(CWD.name, "");
      }

    char full_path[100];

    int i = 0, j = 0;

    // Concatenates where the user wants to go to with the CWD to create a
    // full_path, so it does CWD + path where user wants to go

      while (CWD.name[i] != '\0') {
        full_path[j] = CWD.name[i];
        i++;
        j++;
      }

      if (Lstrcmp(CWD.name, "/") != 0 && Lstrcmp(CWD.name, "") != 0) {
        full_path[j] = '/';
        j++;
      }

      i = 0;

      while (tokens[1][i] != '\0') {
        full_path[j] = tokens[1][i];
        i++;
        j++;
      }

      full_path[j] = '\0';

      // Gets the inode number of the path we want to go to
      uint inode_of_path = namei(tokens[1]);

      // If the path doesn't lead to anything
      if (inode_of_path == 0) {
        Lprintf("The directory you are trying to enter does not exist\n");
        Lprintf("fscli> ");
        for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }
        continue;
      }

      struct dinode inode_data = {};

      // Retrieves the inode data so that we can know whether its a dir or not
      getinode(&inode_data, inode_of_path);

      if (inode_data.type != 1) {
        Lprintf("%s is not a directory\n", tokens[1]);
        Lprintf("fscli> ");

        for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }
        continue;
      }

      //Set the CWD to the new full path
      Lstrcpy(CWD.name, full_path);
      CWD.inum = inode_of_path;

      Lprintf("Current directory: %s\n", CWD.name);
    }

    /*
     ------------------
     --- LS COMMAND---
     ------------------
    */

      if (Lstrcmp(tokens[0], "ls") == 0) {
      int valid = 0;

      // ONLY LS
      if (num_of_tokens == 1) {
        lspath(CWD.name);

        Lprintf("fscli> ");

        for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }

        valid = 1;

        continue;
      }

      char strcopy[50];
      Lstrcpy(strcopy, tokens[1]);

      //ls {pathname}
      if (strcopy[0] == '/' || strcopy[0] != '-') {
        lspath(tokens[1]);

        Lprintf("fscli> ");

        for (int i = 0; i < LINESIZE; i++) {
          buf[i] = '\0';
        }

        valid = 1;

        continue;
      }

     //ls -d
     if (strcopy[0] == '-' && strcopy[1] == 'd') {
        uint inode_num;

        inode_num = namei(num_of_tokens == 2 ? CWD.name : tokens[2]);

        if(inode_num != 0) {
            Lprintf("%s\n", num_of_tokens == 2 ? CWD.name : tokens[2]);
        }

        valid = 1;
    }

     // ls -R
     if (strcopy[0] == '-' && strcopy[1] == 'R') {
        uint inode_num;
        inode_num = namei(num_of_tokens == 2 ? CWD.name : tokens[2]);

        if (inode_num != 0) {
            recursive_ls("", num_of_tokens == 2 ? CWD.name : tokens[2]);
        }

        valid = 1;
     }

     if (valid == 0) {
        Lprintf("ERROR: Incorrect Usage Of 'ls'. Please type 'help' for more information on usage.\n");
     }
    }

    /*
     --------------------
     --- CREAT COMMAND---
     --------------------
    */

    if (Lstrcmp(tokens[0], "creat") == 0) {
		createfunc(tokens[1], 0);
    }

    /*
     ------------------
     --- SYNC COMMAND---
     ------------------
    */

    if (Lstrcmp(tokens[0], "sync") == 0) {
		bsync();
    }

    /*
     ---------------------
     ---  MKDIR COMMAND---
     ---------------------
    */

	if (Lstrcmp(tokens[0], "mkdir") == 0){
		if (num_of_tokens == 2){
			if (namei(tokens[1]) == 0){
				char *dir = tokens[1];
				int pathSize = Lstrlen(dir);
				int i,j,l;
				char strcpy[100] = "";

				//Getting the last word (directory name)
				for (i = pathSize-1, j = 0; dir[i] != '/' && i >= 0; i--, j++){
					strcpy[j] = dir[i];
				} // 'i' now holds the value of the last '/' in the pathname, this will be useful later.

				Lstrrev(strcpy);

				/******* Find a free inode *******/
				struct dinode inode = {};
				uint inum;

				for(int k = 1; k < (BSIZE / sizeof(struct dirent)); k++){
					inum = k;
					int valid = getinode(&inode, inum);

					if(valid == -1) {
						break;
					}
				}

				/***** Get rid of last word *****/
				char parentDir[100] = "";
				uint parentInum;
				struct dinode pinode = {};

				// put the path until the dir name into a new 'string'
				for(l = 0; l < i; l++)
					parentDir[l] = dir[l];

				parentInum = namei(parentDir);
				getinode(&pinode, parentInum);

				uint inodes_per_block = BSIZE / inodesize;

				uint new_inode_block_no = SB.inodestart + inum / inodes_per_block;
				uint parent_inode_block_no = SB.inodestart + parentInum / inodes_per_block;

				struct buf* main_buffer = bread(DEVFD, new_inode_block_no);
				struct buf* free_blocks = bread(DEVFD, SB.bmapstart);

				int free_block = 0;
				for(int i = 0; i < BSIZE; i++) {
					if (free_blocks->data[i] == 0) {
						free_block = 4 * i;
						Lmemset(&free_blocks->data[i], 255, 1);
						break;
					}
				}

				free_blocks->dirty = 1;
				brelse(free_blocks);

				struct dinode new_inode = {};
				new_inode.type = 1;
				new_inode.nlink = 2;
				new_inode.size = 1024;
				new_inode.addrs[0] = free_block * BSIZE;

				Lmemset(&main_buffer->data[(inum % inodes_per_block) * sizeof(struct dinode)], 
						0, sizeof(struct dinode));

  				Lmemcpy(&main_buffer->data[(inum % inodes_per_block) * sizeof(struct dinode)], 
						(const char*)&new_inode, sizeof(struct dinode));

				main_buffer->dirty = 1;
				brelse(main_buffer);

				// PARENT DIRECTORY MANAGEMENT
				//Block where we will write our dirent to
				struct buf* parent_block = bread(DEVFD, pinode.addrs[0]);

				// Read from buffer to then edit the de **
				struct dirent* de;
				int d;

				for (d = 0; d < (BSIZE / sizeof(struct dirent)); d++){
					
				// Read from dirent
    				de = (struct dirent *)(parent_block->data + d * sizeof(struct dirent));
					if(de->inum == 0)
						break;
				}	//d will hold the location in the buffer of this free spot


				//Setting the directory entry to what we want
				struct dirent new_de = {};
				Lstrcpy(new_de.name, strcpy);
				new_de.inum = inum;

				//Updates parent dirents
				Lmemset(&parent_block->data[sizeof(struct dirent) * (d)], 0 , sizeof(struct dirent));
				Lmemcpy(&parent_block->data[sizeof(struct dirent) * (d)], &new_de, sizeof(struct dirent));

				parent_block->dirty = 1;
				brelse(parent_block);

				//Incrementing parent directory's '.'
			    struct buf* parent_buffer = bread(DEVFD, parent_inode_block_no);
				pinode.nlink = pinode.nlink + 1;

  				Lmemcpy(&parent_buffer->data[(parentInum % inodes_per_block) * sizeof(struct dinode)], &pinode, sizeof(struct dinode));
				brelse(parent_buffer);

				//Setting dirents for the child for . and ..
				struct buf* new_dir_block = bread(DEVFD, new_inode.addrs[0]);
				struct dirent de1 = {inum, "."};
				struct dirent de2 = {parentInum, ".."};

				Lmemset(&new_dir_block->data, 0, sizeof(struct dirent));
				Lmemcpy(&new_dir_block->data,(const char*) &de1, sizeof(struct dirent));

				Lmemset(&new_dir_block->data[sizeof(struct dirent)], 0, sizeof(struct dirent));
				Lmemcpy(&new_dir_block->data[sizeof(struct dirent)],(const char*) &de2, sizeof(struct dirent));

				new_dir_block->dirty = 1;
				brelse(new_dir_block);
			}
			else
				Lprintf("ERROR: Directry name already exists\n");
		}
		else
			Lprintf("ERROR: Incorrect Usage of 'mkdir'. Make sure to specify the path and directory name\n");
	}


    /*
     ---------------------
     ---  LINK COMMAND---
     ---------------------
    */

	if (Lstrcmp(tokens[0], "link") == 0) {
		 if(num_of_tokens < 3){
			Lprintf("Invalid syntax. Use link [file] [file] \n");
		} else if (Lstrcmp(tokens[1], ".") == 0 || Lstrcmp(tokens[1], "..") == 0) {
			Lprintf("Cannot link . or .. \n");
		} else {
			//Check if the file we want to link for exists
			//ftbl = file to be linked
			uint inum_of_ftbl = namei(tokens[1]);
			struct dinode inode_of_ftbl = {};
			int err = getinode(&inode_of_ftbl, inum_of_ftbl);

			if (err == -1) {
				Lprintf("The file you are trying to create a link of does not exist\n");
		        Lprintf("fscli> ");
        		for (int i = 0; i < LINESIZE; i++) {
         			 buf[i] = '\0';
        		}

		        continue;
			}

			if (inode_of_ftbl.type != T_FILE) {
				Lprintf("You cannot use link with a directory\n");
		        Lprintf("fscli> ");

        		for (int i = 0; i < LINESIZE; i++) {
         			 buf[i] = '\0';
        		}

		        continue;
			}

			//Find the inode of the file that we want to create a sym link of
			//lf = Linking file
			uint inum_of_lf = namei(tokens[2]);

			if(inum_of_lf > 0){
				Lprintf("The file you are trying to link to already exists \n");
			}

			//Create that file and set its inode to the inode of the file we want to create a sym link of
			createfunc(tokens[2], inum_of_ftbl);
		}
	}

    /*
     ---------------------
     ---  UNLINK COMMAND---
     ---------------------
    */

	if (Lstrcmp(tokens[0], "unlink") == 0) {
		if (num_of_tokens >= 2) {
			uint file_inum = namei(tokens[1]);

			if (file_inum == 0) {
				Lprintf("The file you specified does not exist\n");
			} 
			else {
				struct dinode inode = {};

				//Gets the inode data of the file we want to delete
				getinode(&inode, file_inum);

				//Gets the block where that inode is stored
				uint blockno = SB.inodestart + file_inum / (BSIZE / sizeof(struct dinode));

				//Read the block of the inode into a buffer -> contains the data of the inode file
			 	struct buf* buffer = bread(DEVFD, blockno);

				uint inode_size = sizeof(struct dinode);
				uint inodes_per_block = BSIZE/inode_size;

				if(inode.type == 2){
					// Reduce the hard link count of the inode 
					inode.nlink = inode.nlink - 1;

					if(inode.nlink == 0) {
						Lprintf("File should be deleted\n");
					}

					//Update file's inode in buffer cache
					Lmemcpy(&buffer->data[inode_size * (file_inum % inodes_per_block)], (const char*)&inode, sizeof(struct dinode));
					buffer->dirty = 1;

					//Get the inode of the CWD - the parent to the file we want to delete
					struct dinode cwd_inode_data = {};
					getinode(&cwd_inode_data, CWD.inum);

					//Gets parent directory inode number

					//Reads the block data of the file's directory (containing dirents) 
					struct buf* buffer = bread(DEVFD, cwd_inode_data.addrs[0]);

				    struct dirent* de;

					int inode_position_in_block = 0;
				    for (int d = 0; d < (BSIZE / sizeof(struct dirent)); d++)
				    {
				        // Read from dirent
				        de = (struct dirent*)(buffer->data + d * sizeof(struct dirent));

				        //Checks if the name of the directory entry matches the name of the fil>
				        if (de->inum != 0 && Lstrcmp(de->name, tokens[1]) == 0)
				        {
				          	inode_position_in_block = d;
							de->inum = 0;
				    		buffer->dirty = 1;
							break;
					}
				    }

				    Lmemcpy(&buffer->data[sizeof(struct dirent) *inode_position_in_block], (const char*)de, sizeof(struct dirent));

				    brelse(buffer);
					}
				}
			} 
		else {
			Lprintf("Unlink requires a file \n");
		}
	}

    	// UPLOAD PATH FILENAME COMMAND
	if (Lstrcmp(tokens[0], "upload") == 0 && num_of_tokens == 3){

		// tokens[1] == path of the file you want to upload to host
		// tokens[2] == file name that will appear on host

		upload(tokens[1], tokens[2]);
	}


    // QUIT COMMAND
    if (Lstrcmp(tokens[0], "quit") == 0) {
	bsync();
	return 0;
    }

	// Validate uploadtree command
	if (Lstrcmp(tokens[0], "uploadtree") == 0)
	{
		uploadTree("/usr/test/bin/");
		Lprintf("uploadtree verification command output\n");
	}

	Lprintf("fscli> ");

    // Clearing the buffer
    for (int i = 0; i < LINESIZE; i++) {
      buf[i] = '\0';
    }
}
  Lprintf("\n");
  return 0;
}
