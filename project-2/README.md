## Useful links 

Already implemented functions?: https://github.com/mit-pdos/xv6-riscv/tree/riscv/kernel

## Tasks

- [ ] The command line interface must be able to “walk” any xv6 filesystem image file, and perform the filesystem commands listed below:
- [ ] See ... for specification of the xv6 filesystem. You can use a fixed blocksize of 1024 bytes (1KiB) but, your CLI should be able to handle filesystems of varying total size
    Implement memory buffers for caching the filesystem blocks, but only partially. No need to implement locking and the logger for the xv6 filesystem that are used in the actual xv6 OS.
- [ ] The only file types you need to handle are regular files and directories, ignoring irrelevant fields in the inode (and superblock).
- [ ] Use the filesystem image file fs.img from the xv6-riscv codebase on git hub (git clone or download, then build)
    
While developing your CLI code, it may help to have different filesystem image files, so modify the fs.img file as needed. To modify the filesystem image fs.img file, boot the xv6 OS using QEMU, then create/add/delete files and directories from the booted xv6 OS shell
Organize your code structure by breaking it up into small parts and functions, and commenting well. Writing many small files is OK; avoid writing large files with many or complex C functions. Using make (makefiles) is recommended.

Important: The entire CLI must be written purely in C using only the functions listed in Llibc.h and posix-calls.h. The final target program must be built by linking against Llibc alone and no other library.

![image](https://github.com/Duckateers/OS-Assignment-2/assets/67078991/7f74fd85-4017-4bd7-9b5d-2593a517b69c)

## How to run

Enter source-code directory \
```make run``` \
```./Lcli```\
```
1. Go into source code dir, build the source code Llib files
2. make run achieves this
3. Link the output of built source code to the MIT xv6 fs (./Lcli .../OS-Assignment-2/mitfs/fs.img)

Test the functionality within the clean slate environment (MIT fs)

4. Rebuild the built output file for our source code each time new functionality is added
5. Only possible to have new functionality working if 'Lcli.c' file is modified to account for the new implemented codebase

NOTE: Please create experimental branches when a new function is being implemented
```
