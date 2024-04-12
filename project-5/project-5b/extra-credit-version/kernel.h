/* Kernel functions and variables for use by user programs */

// List of syscalls
int do_kfork(void);         /* syscall fork() (non-standard) */
int do_exit(int exit_code);          /* syscall exit() */
int do_switch(void);        /* syscall task_switch() (voluntary) */
int do_getpid(void);        /* syscall getpid() */
int do_exec(void *progaddr);

int do_wakeup(int event);
int do_sleep(int event);
int do_wait(int *status);


// List of convenience functions
void printTaskQueues(void); /* Print running PROC plus all queues */

