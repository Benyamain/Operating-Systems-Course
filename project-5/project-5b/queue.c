/* Code for enqueueing, dequeueing, and printing linked list of PROCs  */

#include "Llibc/Llibc.h"
#include "type.h"
#include "queue.h"

/* Enqueueing routine for the scheduler */
int
enqueue(PROC **queue, PROC *p)
{
    /* Enter p into queue by priority; FIFO (FCFS) if same prioirty */

    /* Add YOUR CODE here - Programming assignment 5A */

}

/* Dequeueing routine for the scheduler */
PROC
*dequeue(PROC **queue)
{
    /* Remove and return first PROC from queue */

    /* Add YOUR CODE here - Programming assignment 5A */

}

/* Print queue, starting at given PROC */
int
printList(char *name, PROC *p)
{
    Lprintf("%s = ", name);

    /* Print linked list of PROCs as:  [pid priority]-> ... ->NULL */

    /* Add YOUR CODE here - Programming assignment 5A */

}

