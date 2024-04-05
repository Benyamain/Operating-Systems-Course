/********* type.h file for MT system (Wang) **************/

/* riscv64 with 64bit longs & pointers (8 bytes):  gcc/as flag -mabi=lp64 */

#define NPROC     9
#define SSIZE  8192         /* Stack size is 8 * 8192 == 64K */
#define CSIZE    32         /* Context size is 32 * 8 == 256 bytes */

/* Process status (state) */
#define FREE      0
#define READY     1
#define RUNNING   2         /* For clarity only, not needed or used */

//#define STOPPED    3      /* To be used later */
#define SLEEPING   4      /* To be used later */
#define ZOMBIE     5      /* To be used later */

/* Process Control Block (PCB), like linux task_struct */
typedef struct proc {
    struct proc *next;      /* next proc pointer         struct offset 0    */
    long  pid;              /* pid = 0 to NPROC-1        struct offset 8    */
    long  ppid;             /* parent pid                struct offset 16   */
    long  status;           /* PROC status (state)       struct offset 24   */
    long  priority;         /* scheduling priority       struct offset 32   */
    long  uctx[CSIZE];      /* saved CPU regs (context)  struct offset 40   */
    long  ustack[SSIZE];    /* processs stack 64K        struct offset 296  */
} PROC;
/* Total NPROC structs, each of size 5 + CSIZE + SSIZE 64-bit words  */
/* User stack spans the address range &ustack[0] .. &ustack[SSIZE - 1] */
/* Offset of CPU context &uctx[0] within struct PROC is 5 longs == 40 bytes */
// #define OFF_CTX_PCB 40                   /* This is defined in context.h */
