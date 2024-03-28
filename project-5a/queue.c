/* Code for enqueueing, dequeueing, and printing linked list of PROCs  */

#include "Llibc/Llibc.h"
#include "type.h"
#include "queue.h"

/* Enqueueing routine for the scheduler */
int
enqueue(PROC **queue, PROC *p)
{
	PROC *curr, *prev;

    /* Enter p into queue by priority; FIFO (FCFS) if same prioirty */

	/* If the queue is empty or p has higher priority than the head, insert p at the head */
	if (*queue == NULL || p->priority > (*queue)->priority) {
		p->next = *queue;
		*queue = p;
		return 0;
	}

	/* Traverse the queue to find the insertion point */
	prev = NULL;
	curr = *queue;
	while (curr != NULL && curr->priority >= p->priority) {
		prev = curr;
		curr = curr->next;
	}

	/* Insert p after prev */
	p->next = curr;
	prev->next = p;

	return 0;

    /* Add YOUR CODE here */

}

/* Dequeueing routine for the scheduler */
PROC
*dequeue(PROC **queue)
{
    /* Remove and return first PROC from queue */

    /* Add YOUR CODE here */

	PROC *p;

	/* Remove and return first PROC from queue */
	if (*queue == NULL) {
		return NULL;
	}

	p = *queue;
	*queue = (*queue)->next;

	return p;

}

/* Print queue, starting at given PROC */
int
printList(char *name, PROC *p)
{
    Lprintf("%s = ", name);

    /* Print linked list of PROCs as:  [pid priority]-> ... ->NULL */
	while (p!= NULL) {
		Lprintf("[%ld %ld]->", p->pid, p->priority);
		p = p->next;
	}
	Lprintf("NULL\n");

	return 0;

    /* Add YOUR CODE here */

}

