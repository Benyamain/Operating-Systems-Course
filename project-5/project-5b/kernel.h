/* Kernel functions and variables for use by user programs */

// List of syscalls
int do_kfork(void);         /* syscall fork() (non-standard) */
int do_exit(void);          /* syscall exit() */
int do_switch(void);        /* syscall task_switch() (voluntary) */
int do_getpid(void);        /* syscall getpid() */
int do_exec(void *progaddr);


// List of convenience functions
void printTaskQueues(void); /* Print running PROC plus all queues */

