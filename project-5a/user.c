/****************************************************************************

        User program(s) for the MT Multitasking System of Wang

*****************************************************************************/

#include "Llibc/Llibc.h"
#include "kernel.h"

/** List of "user program"(s) in file user.c (just one for now).
    Each user program code is a function - we don't have a filesystem!
    Each must be a function of void returning int, type int (*)(void).  **/
int uprogA(void);
/*
    User programs in MT system only have separate private stack spaces.
    Unlike real processes, they do not have private data spaces,
    so they must not use (private) global or static variables!
*/

/*************************************************************************
    Code for (just one) User Program --- forked user processes share this!
**************************************************************************/
int
uprogA(void)
{

  char *fname = "uprogA";
  int c, CR, ret, mypid;
  int iter = 0, nsyscalls = 0, nviews = 0;

  mypid = do_getpid();
  // WARM-UP ASSIGNMENT: Write the syscall do_getpid()

  Lprintf("\n===>>> proc %ld is starting from function %s() <<<===\n",
    mypid, fname);

  while (1 == 1) {
    iter++;
    Lprintf("***************************************\n");
    Lprintf("proc %ld running (iteration %d, #syscalls=%d, #views=%d)\n",
        mypid, iter, nsyscalls, nviews);
    // Observe how local variables change for various running processes
    Lprintf(" Enter a key [f|s|q (syscall) or v (view tasks)] : ");
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
    } else if (c == 's') {  /* syscall: voluntary task switch */
      nsyscalls++;
      ret = do_switch();
    } else if (c == 'q') {  /* syscall: _exit() */
      nsyscalls++;
      ret = do_exit();
    } else if (c == 'v') {  /* View all running and queued tasks */
      nviews++;
      printTaskQueues();
      // WARM-UP ASSIGNMENT: Write the printTaskQueues() function
    } else {  /* Unknown key entered by user */
      Lprintf(" Usage:  Enter "
        "f (fork) | s (switch task) | q (exit) | v (view)\n");
    }
  }
  return ret;   /* quiet gcc */

}

