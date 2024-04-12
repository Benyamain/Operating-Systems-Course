/****************************************************************************

        User program(s) for the MT Multitasking System of Wang

*****************************************************************************/

#include "Llibc/Llibc.h"
#include "kernel.h"

/** List of "user programs" in file user.c (just two for now).
    Each user program code is a function - we don't have a filesystem!
    Each must be a function of void returning int, type int (*)(void).  **/
int uprogA(void);
int uprogB(void);

/*
    User programs in MT system only have separate private stack spaces.
    Unlike real processes, they do not have private data spaces,
    so they must not use (private) global or static variables!
*/

/*************************************************************************
    Code for User Program A
**************************************************************************/
int
uprogA(void)
{

  char *fname = "uprogA";
  char c, CR;               /* Don't use int, since we are using read() */
  int ret, mypid;
  int iter = 0, nsyscalls = 0, nviews = 0;

  mypid = do_getpid();

  Lprintf("\n===>>>  proc %ld is starting from function %s()  <<<===\n",
    mypid, fname);

  while (1 == 1) {
    iter++;
    Lprintf("***********************************************************\n");
    Lprintf("proc %ld (%s) running:  Iteration %d, #syscalls=%d, #views=%d\n",
        mypid, fname, iter, nsyscalls, nviews);
    // Observe how local variables change for various running processes
    Lprintf("enter a key [f|s|q|a|b (syscall) or v (view tasks)] : ");
    // Read a line, and store the first non-space character in c
    c = ' ';
    while (1 == 1) {
      Lread(0, &CR, 1);
      if (CR == '\n' || CR == '\r')   /* End of line */
        break;
      if (c == ' ' || c == '\t')      /* isspace(c) */
        c = CR;                       /* Record first non-space char in c */
    }
    // c now holds the first non-space char typed, or some space char
    Lprintf("\n");
    if (c == 'f') {         /* syscall: fork() */
      nsyscalls++;
      ret = do_kfork();     /* child if ret == 0, else parent */
      /* Under standardized fork, the child will inherit the variable mypid
         from parent.  Unless mypid is updated via getpid() immediately
         after do_kfork(), mypid's value will remain as the parent's pid. */
      mypid = do_getpid();  /* Without this mypid will hold parent's pid */
    } else if (c == 'a') {  /* syscall:  exec(progA) */
      Lprintf("proc %ld: Got 'a', doing syscall exec(program_A) ...\n", mypid);
      ret = do_exec(&uprogA);
    } else if (c == 'b') {  /* syscall:  exec(progB) */
      Lprintf("proc %ld: Got 'b', doing syscall exec(program_B) ...\n", mypid);
      ret = do_exec(&uprogB);
    } else if (c == 's') {  /* syscall: voluntary task switch */
      nsyscalls++;
      ret = do_switch();
    } else if (c == 'q') {  /* syscall: _exit() */
      nsyscalls++;
      ret = do_exit();
    } else if (c == 'v') {  /* View all running and queued tasks */
      nviews++;
      printTaskQueues();
    } else {  /* Unknown key entered by user */
      Lprintf(" Usage:  Enter "
        "f (fork) | s (switch task) | q (exit) | a,b (exec) | v (view)\n");
    }
  }
  return ret;   /* quiet gcc */

}

/************************************************************************
    Code for second User Program - Manually exits at end of loop!
*************************************************************************/
int
uprogB(void)
{
  char *fname = "uprogB";
  char c, CR;               /* Don't make these int! */
  int ret, mypid;
  int max = 8;

  mypid = do_getpid();

  Lprintf("\n^^==^^  proc %ld starting from %s(), MAX %d RUNS  ^^==^^\n",
        mypid, fname, max);

  for (int k = 0; k < max; k++) {
    Lprintf("***********************************************************\n");
    Lprintf("proc %ld (%s) is in run %d (max %d)\n", mypid, fname, k+1, max);
    Lprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    Lprintf(":)  HELLO WORLD  :)  RUN %d!\n", k+1);
    Lprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    Lprintf("enter a key [f|s|q|a|b (syscall) or v (view tasks)] : ");
    // Read a line, and store the first non-space character in c
    c = ' ';
    while (1 == 1) {
      Lread(0, &CR, 1);
      if (CR == '\n' || CR == '\r')   /* End of line */
        break;
      if (c == ' ' || c == '\t')      /* isspace(c) */
        c = CR;                       /* Record first non-space char in c */
    }
    // c now holds the first non-space char typed, or some space char
    Lprintf("\n");
    if (c == 'f') {         /* syscall: fork() */
      ret = do_kfork();     /* child if ret == 0, else parent */
      /* Under standardized fork, the child will inherit the variable mypid
         from parent.  Unless mypid is updated via getpid() immediately
         after do_kfork(), mypid's value will remain as the parent's pid. */
      mypid = do_getpid();  /* Without this mypid will hold parent's pid */
    } else if (c == 'a') {  /* syscall:  exec(progA) */
      Lprintf("proc %ld: Got 'a', doing syscall exec(program_A) ...\n", mypid);
      ret = do_exec(&uprogA);
    } else if (c == 'b') {  /* syscall:  exec(progB) */
      Lprintf("proc %ld: Got 'b', doing syscall exec(program_B) ...\n", mypid);
      ret = do_exec(&uprogB);
    } else if (c == 's') {  /* syscall: voluntary task switch */
      ret = do_switch();
    } else if (c == 'q') {  /* break and manually exit */
      break;                /* Out of loop will do do_exit() */
    } else if (c == 'v') {  /* View all running and queued tasks */
      printTaskQueues();
    } else {  /* Unknown key entered by user */
      Lprintf(" Usage:  Enter "
        "f (fork) | s (switch task) | q (exit) | a,b (exec) | v (view)\n");
    }
  }

  Lprintf("\n!proc %ld (%s): GOODBYE!!!\n\n", mypid, fname);
  do_exit();
  return ret;   /* quiet gcc */

}

