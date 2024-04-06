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

PROC *sleepList; 	/* Ptr to head of the queue of sleeping processes */

/* File queue.c defines the scheduler's queue-management routines:
    int enqueue(PROC **, PROC *);       // Scheduler enqueueing routine
    PROC *dequeue(PROC **);             // Scheduler dequeueing routine
    int printList(char *, PROC *);      // Print queue, starting at given PROC
*/
#include "queue.h"

/** List of "user programs" from file user.c (just two for now).
    Each user program code is a function - we don't have a filesystem!
    Each must be a function of void returning int, type int (*)(void). **/
int uprogA(void);
int uprogB(void);

/* Functions defined in this file (kernel.c) */
//
// Syscalls
int do_kfork(void);             /* fork() syscall NOW STANDARDIZED! */
int do_exit(void);              /* exit() syscall */
int do_switch(void);            /* task_switch() syscall (voluntary) */
int do_getpid(void);            /* getpid() syscall */
int do_exec(void *progaddr);    /* exec() syscall */

//
// Convenience functions
void printTaskQueues(void);     /* show running PROC and all queues */
//
// Kernel implementation routines for syscalls and other functions
int initctx(PROC *p, void *progaddr);   /* Initialize a PROC context area */
int kfork(void);                /* kernel rouine for fork() STANDARDIZED! */
int kexit(void);                /* kernel routine for _exit() */
int kexec(void *progaddr);      /* kernel exec */

int ksleep(int event);
int kwakeup(int event);
int kwait(int *status);

int scheduler(void);            /* kernel scheduler */
int queinit(void);              /* system que and PROC 0 initialization */
// Assembly routines
int savectxfork(void);          /* Save context and call kfork() (in asm) */
int tswitch(void);              /* kernel task-switch routine (in asm) */
int runctx(void);               /* Run/resume from context (in asm) */


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
  Re-write to implement STANDARD UNIX FORK!
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

  /* Start setting up the new child process p */
  p->status = READY;
  p->priority = 1;          // priority = 1 for ALL PROCs, except P0
  p->ppid = running->pid;   // Current running PROC is the parent

  /******************************************************************
    When a PROC is dequeued from freeList, its uctx[] and ustack[]
    will contain garbage so we must initialize them meaningfully.

    In standard unix fork, they are made duplicates of the parent's
    context and stack - this is part of programming assignment 5B.

    In standard unix fork:
      - The child's code, stack, CPU registers will be same as parent's
      - The child, like the parent, will begin execution from immediately
        after the point where the parent had called fork()
      - The only ways the child will differ from the parent will be
        that its PID will be different, and when scheduled to run
        the child will see a return value of 0 from fork(), while
        the parent will see the child's PID as the return value
  *****************************************************************/

  /* Add YOUR CODE here!

    Hints:
      - Don't re-initilize p's context to the program's starting state
      - Copy child'ss stack and context from the parent
      - But there will be two things wrong in the child's context
        that you will need to fix up:
        (1) The saved stack pointer in the child's context will be
            pointing to inside the parent's stack - fix this!
        (2) The child, when scheduled to run, should see a return
            value of 0 from the fork syscall, so the function return
            value register in the context will also need to be updated
            (recall riscv function calling convention)
  */

	/* Copy child's stack and context from the parent */
	//Lmemcpy(p->ustack, running->ustack, sizeof(p->ustack));
	//Lmemcpy(p->uctx, running->uctx, sizeof(p->uctx));

	/* Fix up the child's context */
	//p->uctx[2] = (long) &(p->ustack[SSIZE - 1]);	// Update sp in the pcb that holds register x2
	//p->uctx[10] = 0;	// Return value for the child (a0) is register x10


  /* Enter p (child) into readyQueue */
  enqueue(&readyQueue, p);

  return p->pid;

}

int
kexec(void *progaddr)
{

  /* Add YOUR CODE here! */
	/* Re-initialize the current process's context */
	//initctx(running, progaddr);

	/* Transfer control to the new program */
	//runctx();

	/* Should not be reached */
	//return -1;

}

/******************************************
  kernel routine kexit() (no zombie)
*******************************************/
/* Terminates the running process and wakes up its parent if waiting */
int
kexit(void)
{
  running->status = FREE;
  running->priority = 0;
  enqueue(&freeList, running);              // Enter running into freeList

  Lprintf(" K: -------------------------------------\n");
  Lprintf(" K: proc %ld: TERMINATED!\n", running->pid);
  Lprintf(" K: -------------------------------------\n");
  printList("     freeList", freeList);     // Show freeList

  Lprintf(" K: Switching task via tswitch() ..\n");
  return tswitch();                         // Call task switcher
}

/* Terminates the running process and wakes up its parent if waiting */
/*int
kexit(int exitCode)
{
	running->exitCode = exitCode;
	running->status = ZOMBIE;
	Lprintf(" K: -------------------------------------\n");
	Lprintf(" K: proc %ld: TERMINATED!\n", running->pid);
	Lprintf(" K: -------------------------------------\n");
	printList("     freeList", freeList);

	// Wake up parent if it's waiting
  	kwakeup(running->ppid);

  	return tswitch();
}*/

/* system call fork() */
int
do_kfork(void)
{
  int child = savectxfork();
  /* This point, immediately after call to savectxfork(), will be
     returned into (reached) twice!  First from the parent, which
     creates and enqueues the child and then returns its pid here,
     and then later when the created child first gets its turn to run,
     it will also return here because in its context the saved ra
     slot has this address.
  */
  if (child < 0) {
    Lprintf(" K: kfork failed!\n");
  } else if (child == 0) {              /* child */
    Lprintf(" K: child created after fork, pid = %d, becoming live to run\n",
      running->pid);
  } else {                              /* child > 0, so parent */
    Lprintf(" K: proc %ld kforked a child, pid = %d\n", running->pid, child);
    printList("     readyQueue", readyQueue);
  }
  return child;
}

/* Puts the running process to sleep waiting for a specific event */
/*int
ksleep(int event)
{
 	running->status = event;
	//running->status = SLEEPING;
	enqueue(&sleepList, running);
	return tswitch();
}*/

/* Wakes up all processes waiting for a specific event  */\
/*int
kwakeup(int event)
{
	PROC *p, *tmp;
	p = sleepList;

	while (p) {
		if (p->status == event) {
			tmp = p;
			p = p->next;
			dequeue(&sleepList);
			enqueue(&readyQueue, tmp);
		}
		else {
			p = p->next;
		}
	}

	return 0;
}*/

/* Waits for a child process to exit and retrieves its exit status  */
/*int
kwait(int *status)
{
	PROC *p;

	while (1 == 1) {
		for (p = proc; p < &proc[NPROC]; p++) {
			if (p->ppid == running->pid && p->status == ZOMBIE) {
				*status = p->priority;	// Use priority to store exit code
				p->status = FREE;
				p->priority = 0;
				enqueue(&freeList, p);

				return p->pid;
			}
		}

		ksleep(running->pid);	// Use process ID as the event for waiting
	}
}*/

/* system call exec() */
int
do_exec(void *progaddr)
{
  Lprintf(" K: exec for proc %ld\n", running->pid);
  if (running->pid == 1){   /* Do not allow exec from P1, avoid system crash */
    Lprintf(" K: P1 does not allow exec!\n");
    return -1;
  }
  return kexec(progaddr);          /* Should not return */
  /* Should not be here */
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
     held live in the ra register when going into tswitch(), which will
     save this address into the saved ra slot uctx[1] in the outgoing
     PROC's context.  When the next line is reached after return from
     tswitch(), it will typically be into a diffrent PROC, since
     meanwhile the scheduler has changed the PROC!
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
  /* Add YOUR CODE here - one line can be enough! (Assignment 5A) */
	/* Return the PID of the currently running process */
	return running->pid;
}

/* system call sleep() */
/*int
do_sleep(int event)
{
	return ksleep(event);
}*/

/* system call wakeup() */
/*int
do_wakeup(int event)
{
	return kwakeup(event);
}*/

/* system call wait() */
/*int
do_wait(int *status)
{
	return kwait(status);
}*/

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

	/* Set head for the sleep queue */
	//sleepList = (PROC *) 0;

  /* Create P0 as the initial running process */
  p = dequeue(&freeList);       /* This p is P0 */
  p->status = READY;
  p->ppid = 0;                  /* P0 is its own parent */

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
     and creates the dummy initial process P0.  P0 "runs" only for a flash
     at the beginning as the first outgoing process, and will never run again!
  */
  queinit();        /* Initialize the process queues; create and run P0 */
  printList("     readyQueue", readyQueue);
  printList("     freeList", freeList);
  Lprintf(" Continue startup: kfork a real user process into readyQueue ..\n");
  /* Standardized kfork() doesn't initialize the child (just clones
     from the parent), so if the newly forked P1 from P0 is to be
     runnable, then we need to initialize P0 before forking off P1!
     Otherwise, P0's PCB will be all null, so when P1 is created
     from P0 via standardized kfork(), it will also have a null PCB!
  */
  initctx(running, &uprogA);        /* Initialize P0 context */
  /* Now P1 created from P0 via standardized kfork() can run! */

  kfork();     /* kfork P1 into readyQueue, with still running->P0 */

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

    /* Programming Assignment 5A */
	Lprintf("     --- printing current task and queues ---\n");
	Lprintf("     Now running -> [%ld %ld] (PID %ld, Priority %ld, PPID %ld)\n", do_getpid(), running->priority, do_getpid(), running->priority, running->ppid);
	printList("     readyQueue", readyQueue);
	printList("     freeList", freeList);

}


/*************** References ***********************

1. Design and Implementation of the MTX Operating System
by Wang, K.C. Springer (2015), ISBN: 978-3-319-17574-4.

2. Systems Programming in Unix/Linux,
by Wang, K.C. Springer (2018), ISBN: 978-3-319-92428-1.

***************************************************/

