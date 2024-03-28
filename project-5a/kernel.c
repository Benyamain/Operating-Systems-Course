/****************************************************************************
        The MT Multitasking System of Wang (see references below)

  This file contains the OS kernel code.

  The user program(s) are in file user.c.
*****************************************************************************/

#include "Llibc/Llibc.h"
#include "type.h"   /* Defines struct PROC == Process Control Block (PCB) */

/* Global variables */
PROC proc[NPROC];   /* Allocate storage for NPROC PCBs (the process table) */
PROC *running;      /* Ptr to PCB of current running process (at most one) */
PROC *freeList;     /* Ptr to head of the queue (linked list) of free PIDs */
PROC *readyQueue;   /* Ptr to head of the queue of ready to run processes */

/* File queue.c defines the scheduler's queue-management routines:
    int enqueue(PROC **, PROC *);       // Scheduler enqueueing routine
    PROC *dequeue(PROC **);             // Scheduler dequeueing routine
    int printList(char *, PROC *);      // Print queue, starting at given PROC
*/
#include "queue.h"

/** List of "user program"(s) from file user.c (just one for now).
    Each user program code is a function - we don't have a filesystem!
    Each must be a function of void returning int, type int (*)(void). **/
int uprogA(void);

/* Functions defined in this file (kernel.c) */
//
// Syscalls
int do_kfork(void);             /* fork() syscall (non-standard) */
int do_exit(void);              /* exit() syscall */
int do_switch(void);            /* task_switch() syscall (voluntary) */
int do_getpid(void);            /* getpid() syscall */
//
// Convenience functions
void printTaskQueues(void);     /* show running PROC and all queues */
//
// Kernel implementation routines for syscalls and other functions
int initctx(PROC *p, void *progaddr);   /* Initialize a PROC context area */
int kfork(void);                /* kernel rouine for fork() (non-standard) */
int kexit(void);                /* kernel routine for _exit() */
int tswitch(void);              /* kernel task-switch routine (in asm) */
int scheduler(void);            /* kernel scheduler */
int queinit(void);              /* system que and PROC 0 initialization */


/*********************************************************************
  initctx() initializes the context p->uctx[] of a given PROC p.  It will
    - Set the saved ra context-slot p->uctx[1] to the given prog addr
    - Set the saved sp context-slot p->uctx[2] to stack-beginning addr
  The idea is that if a process is resumed from such a context and run
  using `ret' (assembly routine in ts-riscv64.S), then it will start
  executing from the given program addr with all of stack available.
*********************************************************************/
int
initctx(PROC *p, void *progaddr)
/* 
  We have simplified the type of the second argument progaddr as `void *',
  but it really should be `int (*progaddr)(void)', since the address of
  a user prog function is a pointer to a function of void returning int.
*/
{
  /***** riscv64 task context area uctx[] and stack contents *****

    The "context" array uctx[32] = {x0,x1,x2,...,x31} is for holding
    saved CPU registers.  Each uctx[k] is a long (64bits == 8 bytes).

    uctx[] contents:     uctx[0]   uctx[1]   uctx[2]         uctx[31]
                       | x0 = 0 | x1 = ra | x2 = sp | . . . |  x31   |
                         zero      retPC    stk-ptr
    uctx byte offset:      0         8        16      . . .    248
    PROC byte offset:     40        48        56      . . .    288

    The byte offset of uctx[k] from uctx[0] is 8*k (multiply index by 8).

  The function initctx will do the following to p's context area p->uctx[]:

    (1) Clear the entire context (register slots are set to 0);
    (2) Set the saved ra register slot uctx[1] (holding retPC addr)
        to the supplied program address value progaddr;
    (3) Set the saved sp slot uctx[2] to the stack start-address
        (== &ustack[SSIZE-1], stack bottom), effectively
        re-initializing the stack.

  **********************************************************/

  /* Zero out all saved register slots in p's context */
  for (int i = 0; i < CSIZE; i++)
    p->uctx[i] = 0;

  /* Set return addr value (saved ra slot) := given program address */
  p->uctx[1] = (long) progaddr;

  /* Initialize stack ptr (saved sp slot) to stack-beginning (stack bottom) */
  p->uctx[2] = (long) &(p->ustack[SSIZE-1]);

  /*
    If p is now resumed from such a context and run using the `ret'
    instruction, it will start from the return addr value stored in the
    saved ra slot p->uctx[1]:  Execution will start from the specific
    program address supplied here (progaddr), with stack pointer
    initialized to the beginnning of ustack (stack bottom).
  */

  return 0;
}


/****************************************************************************
  kernel routine kfork():  Create and enqueue a child PROC, return child pid.
  But this differs from standard unix fork():  When scheduled to run, the
  child is re-started, i.e. it resumes from beginning of user program code!
*****************************************************************************/
int
kfork(void)
{
  PROC *p;

  /*** Get a proc from freeList to create the child proc ***/
  p = dequeue(&freeList);
  if (!p) {
    Lprintf(" K: NO MORE PROC SLOT AVAILABLE!\n");
    return(-1);
  }

  /* Initialize the new child process p */
  p->status = READY;
  p->priority = 1;          // priority = 1 for ALL PROCs, except P0
  p->ppid = running->pid;   // Current running PROC is the parent

  /******************************************************************
    When a PROC is dequeued from freeList, its uctx[] and ustack[]
    will contain garbage so we must initialize them meaningfully.
    In standard unix fork, they are made duplicates of the parent's
    context and stack (part of the next programming assignment).

    Our kfork() here differs from standard unix fork:  We set up
    the child to begin execution from the start of its program code,
    instead of resuming from the point where the parent had called fork().
    We call `initctx(p, &uprogA)' to re-initialize the child process
    p's context, and then enqueue it as a ready-to-run process.
    The `initctx(p, &uprogA)' sets the saved ra value p->uctx[1]
    (return addr) to the starting addr of the program &uprogA, and
    the saved sp value p->uctx[2] (stack ptr) to the beginning of
    ustack (stack-bottom).  So when the child will be scheduled
    to run, it will resume from the beginning of its program code,
    with the stack pointer initialized to the beginning of stack.
    So the kfork() here RESTARTS THE CHILD (like `fork + self-exec').

    (Since we are re-initializing the child and the child is not
    live yet, we could/should zero out its ustack, but we skip it.
    Without a kernel stack the zeroing-out would be messy to do
    when implementing exec, which is where it really matters.
    In a real OS kernel, exec must zero the ustack for security.)

    It will be an assignment to implement separate kfork and kexec
    kernel routines to make kfork behave like standard unix fork.
  *****************************************************************/

  /* Re-initilize p's context to the program's starting state */
  initctx(p, &uprogA);

  /* Enter p (child) into readyQueue */
  enqueue(&readyQueue, p);

  return p->pid;

}

/******************************************
  kernel routine kexit() (no zombie)
*******************************************/
int
kexit(void)
{
  running->status = FREE;
  running->priority = 0;
  enqueue(&freeList, running);              /* Enter running into freeList */

  Lprintf(" K: -------------------------------------\n"); 
  Lprintf(" K: proc %ld: TERMINATED!\n", running->pid);
  Lprintf(" K: -------------------------------------\n");
  printList("     freeList", freeList);     /* Show freeList */

  Lprintf(" K: Switching task via tswitch() ..\n");
  return tswitch();                         /* Call task switcher */
}

/* system call fork() */
int
do_kfork(void)
{
  int child = kfork();
  if (child < 0) {
    Lprintf(" K: kfork failed!\n");
  } else {
    Lprintf(" K: proc %ld kforked a child, pid = %d\n", running->pid, child); 
    printList("     readyQueue", readyQueue);
  }
  return child;
}

/* system call for task_switch() (voluntary switch) */
int
do_switch(void)
{
  int ret;

  Lprintf( 
    " K: proc %ld switching out voluntarily, calling tswitch() ..\n",
    running->pid);   

  ret = tswitch();      /* Context save-restore done in assembly code */

  /* The address of the next line (Lprintf) below is the return address
     held live in the ra register when going into tswitch().  The SAVE
     assembly routine in tswitch() will save this address into the saved
     ra slot uctx[1] in the outgoing PROC's context.  When the next line
     is reached after return from tswitch(), it will typically be into
     a diffrent PROC, since meanwhile the scheduler has changed the PROC!
  */

  Lprintf(" K: proc %ld resuming, tswitch() has returned ..\n",
    running->pid, running->pid);

  return ret;
}

/* _exit() system call */
int
do_exit(void)
{
  if (running->pid == 1){
    Lprintf(" K: P1 never dies\n");
    return -1;
  }
  return kexit();   // Journey of no return ...
}

/* getpid() syscall */
int
do_getpid(void)
{
  /* Add YOUR CODE here - one line can be enough! */
	/* Return the PID of the currently running process */
	return running->pid;
}

/********************************************************************
  Initialization routine for the MT system OS kernel:
    - Set up the two queues (linked lists):  freeList and readyQue
    - Create P0 as an initial dummy running process
*********************************************************************/
int
queinit(void)
{
  int i;
  PROC *p;

  /* Initialize a linked list of all PROCs:  PCB0 -> PCB1 -> ... -> NULL */
  for (i = 0; i < NPROC; i++) {
    p = &proc[i];       /* p now points to the i-th PCB in the table */
    p->pid = i; 
    p->status = FREE;
    p->priority = 0;
    p->next = p+1;      /* I.e. (PROC *) &proc[i+1] (beauty of C pointers) */
  }
  proc[NPROC-1].next = (PROC *) 0;  /* Terminating null ptr */
 
  /* Set heads for the two queues (linked lists), freeList and readyQueue */
  freeList = &proc[0];          /* Put all PROCs in freeList, with head = P0 */
  readyQueue = (PROC *) 0;      /* The readyQueue is empty */

  /* Create P0 as the initial running process */
  p = dequeue(&freeList);       /* This p is P0 */
  p->status = READY;
  p->ppid = 0;                  /* P0 is its own parent */
  /* P0 won't run any user code, so no need to set up its context/stack */

  running = p;                  /* Set running -> P0 */

  Lprintf(" queinit() done: Queues initialized, "
            "initial dummy PROC P0 running -> [%ld %ld]\n",
                running->pid, running->priority);

  return 0;
}

/*********************************************************
    The main startup caller "boots" the MT system
**********************************************************/
int
Lmain(int argc, char *bootparms[])
{
  int ret;

  Lprintf("\nWelcome to the MT Multitasking System, starting up ..\n");

  /* Call queinit(), which initializes the two queues (freeList, readyQueue)
     and creates the dummy initial process P0.  P0 has no code!  It "runs"
     only for a flash at the beginning as the first outgoing process, and
     will never run again!
  */
  queinit();        /* Initialize the process queues; create and run P0 */
  printList("     readyQueue", readyQueue);
  printList("     freeList", freeList);
  Lprintf(" Continue startup: kfork a real user process into readyQueue ..\n");
  kfork();     /* kfork P1 into readyQueue */
  printList("     readyQueue", readyQueue);
  printList("     freeList", freeList);
  /* P0 will now go out and will never be seen again! */
  Lprintf(" P0 now being task-switched out via tswitch(), P1 coming in ..\n");
  ret = tswitch(); /* dequeues P1 out of readyQueue and makes it running */
  /* Should never reach this point */
  Lprintf(" !All dead! MT system ending!\n");
  return ret;
}

/*********** The scheduler *************/
int
scheduler(void)
{ 
  /* Log scheduler action steps verbosely */

  Lprintf(" K: Entering scheduler() with cur running -> [%ld %ld]\n",
    running->pid, running->priority);
  printList("     readyQueue", readyQueue);

  /* Enqueue outgoing PROC if status == READY, else skip */
  if (running->status == READY) {
    Lprintf("      proc %ld is outgoing, being enqueued by scheduler()\n",
      running->pid);
    enqueue(&readyQueue, running);  /* SCHEDULER ACTION! */
  } else {
    Lprintf("      proc %ld is not runnable, not enqueued\n", running->pid);
  }
  printList("     readyQueue", readyQueue);

  /* Dequeue incoming PROC */
  running = dequeue(&readyQueue);   /* SCHEDULER ACTION! */
  Lprintf(
      "      next running (incoming) dequeued from readyQueue: [%ld %ld]\n",
        running->pid, running->priority);
  printList("     readyQueue", readyQueue);
  Lprintf(" K: Leaving scheduler() with new running -> [%ld %ld]\n",
    running->pid, running->priority);

  return running->pid;
}

/* Print running PROC and all queues */
void
printTaskQueues(void)
{
    /* Add YOUR CODE here!  Match with output of kernel.prebuilt */
	/*Lprintf("--- printing current task and queues ---\n");
	Lprintf("	Now running -> [%ld %ld] (PID %ld, Priority %ld, PPID %ld)\n", do_getpid(), running->priority, do_getpid(), running->priority, running->ppid);
	printList("	readyQueue", readyQueue);
	printList("	freeList", freeList);*/

    /* Hint: You can use the printList() function here even before
       you write its code in queue.c; the pre-compiled queue.o
       contains the printList() function (without source code) */

}


/*************** References ***********************

1. Design and Implementation of the MTX Operating System
by Wang, K.C. Springer (2015), ISBN: 978-3-319-17574-4.

2. Systems Programming in Unix/Linux,
by Wang, K.C. Springer (2018), ISBN: 978-3-319-92428-1.

***************************************************/

