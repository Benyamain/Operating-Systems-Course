/* Queue-management functions defined in queue.c */

int enqueue(PROC **, PROC *);       /* Scheduler enqueueing routine */
PROC *dequeue(PROC **);             /* Scheduler dequeueing routine */
int printList(char *, PROC *);      /* Print queue, starting at given PROC */

