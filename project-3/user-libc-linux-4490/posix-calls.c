/*
    All function names are prefixed with 'L'!

    POSIX syscall wrappers for linux in standalone code (matches xv6 list),
    which can be used directly without C startup routines (without main).

    The linux style syscall(2) function is implemented in syscall-riscv64.S.

    Updated 2024 February 13 by Abhijit Dasgupta (MIT license).

*/

#include <stdarg.h>         /* From gcc really, not C library  */

#include "posix-calls.h"

int
Lfork(void)
{
    long ctidp;     /* Child thread id pointer for clone */
    /* The folllowing needs #include <linux/sched.h> */
    return Lsyscall(SYS_clone,
        CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD /* ulong flags*/,
        0 /* child stack */,
        0 /* ? int *parent_tid ? */,
        0 /* ? unsigned long tls ? */,
        &ctidp /* child_tidptr - should we set this to 0? */);

    /*  There are a lot of architecture dependent variations for this
        kernel level clone syscall, described in the clone(2) man page!
        The riscv64 form seems to match the following:

            long clone(unsigned long flags, void *stack,
                     int *parent_tid, unsigned long tls,
                     int *child_tid);
    */
}

void
Lexit(int status)
{
    Lsyscall(SYS_exit, status, 0, 0, 0);
}

int
Lwait(int *status)
{
    return Lsyscall(SYS_wait4, -1, status, 0, 0);
}

int
Lpipe(int fd[2])
{
    return Lsyscall(SYS_pipe2, fd, 0);
}

long int
Lread(int fd, void *buf, long unsigned int count)
{
    return Lsyscall(SYS_read, fd, buf, count);
}

long int
Lwrite(int fd, const void *buf, long unsigned int count)
{
    return Lsyscall(SYS_write, fd, buf, count);
}

long int
Llseek(int fd, long int offset, int whence)
{
    return Lsyscall(SYS_lseek, fd, offset, whence);
}

int
Lclose(int fd)
{
    return Lsyscall(SYS_close, fd);
}

int
Lkill(int pid)
{
    return Lsyscall(SYS_kill, pid, 15);
}

int
Lexec(char *prog, char **args)
{
    return Lsyscall(SYS_execve, prog, args, 0);
}

int
Lexecve(char *prog, char **args, char **envp)
{
    return Lsyscall(SYS_execve, prog, args, envp);
}

/* At least on riscv64 linux, O_TMPFILE is not fully defined in fcntl.h */
#ifndef O_TMPFILE
#ifdef __O_TMPFILE
#define O_TMPFILE __O_TMPFILE
#endif
#endif
static int
Lvopen(const char *path, int flags, va_list argp)
{
    // va_list argp;            /* Caller passes this */
    // va_start(argp, format);  /* Caller will do this ... */
    mode_t mode = 0;

    /*  How to check if Lopen also had a third arg/param (mode):
        Third param of Lopen is significant if and only if 
        the O_CREAT or O_TMPFILE bit was set in the second param (flags).
        This method for checking if there are more args to be processeed
        is needed in situations like this.  See:
        www.gnu.org/software/libc/manual/html_node/How-Many-Arguments.html
    */
    if (((flags & O_CREAT) != 0)
        #ifdef O_TMPFILE
         || ((flags & O_TMPFILE) != 0)
        #endif
        )
        mode |= va_arg(argp, mode_t);
    return Lsyscall(SYS_openat, AT_FDCWD, path, flags, mode);
}

int
Lopen(const char *path, int flags, ...)
{
    // return Lsyscall(SYS_openat, AT_FDCWD, path, flags, 0);
    // Old way, mode forced null, and so no support for O_CREAT 

    va_list argp;

    va_start(argp, flags);
    return Lvopen(path, flags, argp);
    va_end(argp);

}

int
Lcreat(const char *pathname, mode_t mode)
{
    return Lopen(pathname, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

int
Lmknod(char *path, short dev, short mode)
{
    return Lsyscall(SYS_mknodat, AT_FDCWD, path, (int) mode, (int) dev);
}

int
Lunlink(const char *path)
{
    return Lsyscall(SYS_unlinkat, AT_FDCWD, path, 0);
}

int
Lfstat(int fd, struct stat *statbuf)
{
    return Lsyscall(SYS_fstat, fd, statbuf);
}

int
Llink(const char *oldpath, const char *newpath)
{
    return Lsyscall(SYS_linkat, AT_FDCWD, oldpath, AT_FDCWD, newpath);
}

int
Lmkdir(char *path)
{
    return Lsyscall(SYS_mkdirat, AT_FDCWD, path, 0777);
}

int
Lchdir(const char *path)
{
    return Lsyscall(SYS_chdir, path);
}

int
Lgetcwd(char *buf, unsigned int len)
{
    return Lsyscall(SYS_getcwd, buf, len);
}

int
Ldup(int oldfd)
{
    return Lsyscall(SYS_dup, oldfd);
}

int
Ldup2(int oldfd, int newfd)
{
    return Lsyscall(SYS_dup3, oldfd, newfd, 0);
}

int
Lgetpid(void)
{
    return Lsyscall(SYS_getpid);
}

int Lpause(void)
{
    return Lsyscall(SYS_ppoll, 0, 0, 0, 0,
                                8 /* sizeof(kernel_sigset_t) for alpha */);
}

unsigned int
Lsleep(unsigned int decisecs)   /* Match xv6 */
{
    struct timespec ts;
    ts.tv_sec = decisecs/10;   /* seconds */
    ts.tv_nsec = (decisecs % 10) * 100000000;     /* nanoseconds */
    return Lsyscall(SYS_nanosleep, &ts, 0);
}

int
Luptime(void)       /* Not implemented */
{
    return 0;
}

/* Simple sbrk allocator without freeing mechanism */
#define HEAPMAX (16*1024*1024)          // 16MiB
static char *heap = 0;
static char *brkp = 0;
static char *endp = 0;

void *
Lsbrk(long int size)
{
    if (heap == 0 && brkp == 0 && endp ==0) {   /* First call to sbrk */
        // Pre-allocate the HEAPMAX (16MiB) COW chunk, no real free :(
        heap = (char *) Lsyscall(SYS_mmap,
                            0,
                            HEAPMAX,
                            (PROT_READ | PROT_WRITE),
                            (MAP_PRIVATE | MAP_ANONYMOUS),
                            -1,
                            0);
        brkp = heap;
        endp = brkp + HEAPMAX;
    }

    if (size == 0) {
        return (void *) brkp;
    }

    void *newbeg = (void *) brkp;
    brkp += size;
    if (brkp >= endp) {
        return 0;
    }
    if (brkp < heap) {
        brkp = heap;
        return 0;
    }
    return newbeg;
}

