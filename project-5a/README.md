Running the fork() process indicates for the queue to store that, being FIFO. 

It does not run this process until the task switch is pressed. 

The task switch is stored in the queue as last, executes the first process, then the fork task is deleted once the first process is done. But the process is still in the queue. 

The first pid, P1, never dies; this is when the program first runs and the user has not pressed anything. 

There is the freeList and the readyQueue … in the freeList, this is what you have: freeList = [1 0]->[2 0]->[3 0]->[4 0]->[5 0]->[6 0]->[7 0]->[8 0]->NULL. Then, without the user touching the program, readyQueue = NULL, becomes readyQueue = [1 1]->NULL and freeList is short one now. 

Also pay attention that the dummy initial process [0 0] is what gets run first and does a fork, then a task switch to go to [1 1] (a real process taken from the freeList). Once the fork and task switch is done, the current process becomes [1 1], indicating that the dummy process was stored back into the readyQueue (but not killed). And from there, that is the point where the user can see all of this and can now interact with the terminal. 

There is a limited amount of processes you can run, that can only go up to 8 (you know this when checking for the freeList = NULL). 

When tokenizing the input and checking, be sure to understand that user can only be able to have valid input, with them being 'f', 's', 'q', 'v', or ctrl+c to quit the program. Each key that is pressed (besides ctrl+c) prints some relevant information for that action. 

Each time that you view the current state, the iteration and views increase (this is more complicated because there are other actions that also change this, but for now, just make note of it). 

So, for each time there is a task switch, the program takes the current process that is running, puts it at the very back of the readyQueue, and the process that is incoming becomes the current process. You can keep task switching infinitely in there. The special case is at the start of the program if user does a task switch immediately without forking, where only a real process [1 1] is present. The program will just act like nothing happened and the current process will still be [1 1] because there are no other real processes present ([0 0] is a dummy process that cannot be accessed again). Keep note that a hint was dropped, indicating that a periodic timer maybe needs to be used to make sure the task switching can occur in the background.  

Because we can only use 8 processes, stored in either the readyQueue or freeList, we can tell which one has a process attached to it. For instance, the freeList has the PID stored in a 2D array and a bool of 0 that exemplifies no process is attached to the PID. Once fork() is called, then will it only take the head of its freeList queue, mark it as having a process now, being a bool of 1, then adding it to the readyQueue. 

 

On start: 

 

Welcome to the MT Multitasking System, starting up .. 

queinit() done: Queues initialized, initial dummy PROC P0 running -> [0 0] 

     readyQueue = NULL 

     freeList = [1 0]->[2 0]->[3 0]->[4 0]->[5 0]->[6 0]->[7 0]->[8 0]->NULL 

Continue startup: kfork a real user process into readyQueue .. 

     readyQueue = [1 1]->NULL 

     freeList = [2 0]->[3 0]->[4 0]->[5 0]->[6 0]->[7 0]->[8 0]->NULL 

P0 now being task-switched out via tswitch(), P1 coming in .. 

K: Entering scheduler() with cur running -> [0 0] 

     readyQueue = [1 1]->NULL 

      proc 0 is outgoing, being enqueued by scheduler() 

     readyQueue = [1 1]->[0 0]->NULL 

      next running (incoming) dequeued from readyQueue: [1 1] 

     readyQueue = [0 0]->NULL 

K: Leaving scheduler() with new running -> [1 1] 



===>>> proc 1 is starting from function uprogA() <<<=== 

*************************************** 

proc 1 running (iteration 1, #syscalls=0, #views=0) 

Enter a key [f|s|q (syscall) or v (view tasks)] : 

 

For forking (f): 

 

K: proc 1 kforked a child, pid = 2 

     readyQueue = [2 1]->[0 0]->NULL 

*************************************** 

proc 1 running (iteration 2, #syscalls=1, #views=0) 

Enter a key [f|s|q (syscall) or v (view tasks)] : 

 

For task switching (s): 

 

K: proc 1 switching out voluntarily, calling tswitch() .. 

K: Entering scheduler() with cur running -> [1 1] 

     readyQueue = [2 1]->[0 0]->NULL 

      proc 1 is outgoing, being enqueued by scheduler() 

     readyQueue = [2 1]->[1 1]->[0 0]->NULL 

      next running (incoming) dequeued from readyQueue: [2 1] 

     readyQueue = [1 1]->[0 0]->NULL 

K: Leaving scheduler() with new running -> [2 1] 

 

For killing the current process (q): 

 

K: ------------------------------------- 

K: proc 2: TERMINATED! 

K: ------------------------------------- 

     freeList = [3 0]->[4 0]->[5 0]->[6 0]->[7 0]->[8 0]->[2 0]->NULL 

K: Switching task via tswitch() .. 

K: Entering scheduler() with cur running -> [2 0] 

     readyQueue = [1 1]->[0 0]->NULL 

      proc 2 is not runnable, not enqueued 

     readyQueue = [1 1]->[0 0]->NULL 

      next running (incoming) dequeued from readyQueue: [1 1] 

     readyQueue = [0 0]->NULL 

K: Leaving scheduler() with new running -> [1 1] 

K: proc 1 resuming, tswitch() has returned .. 

*************************************** 

proc 1 running (iteration 3, #syscalls=2, #views=0) 

Enter a key [f|s|q (syscall) or v (view tasks)] : 

 

For viewing the current state of processes, freeList, readyQueue (v): 

 

--- printing current task and queues --- 

     Now running -> [1 1] (PID 1, Priority 1, PPID 0) 

     readyQueue = [0 0]->NULL 

     freeList = [3 0]->[4 0]->[5 0]->[6 0]->[7 0]->[8 0]->[2 0]->NULL 

*************************************** 

proc 1 running (iteration 4, #syscalls=2, #views=1) 

Enter a key [f|s|q (syscall) or v (view tasks)] : 



How do we know what fork process becomes a child of another? When the task switch runs on the current process fork, and we create a new fork process while under that current process!! 

How is it possible to kill the current process PID (marked by the circle), and have a child underneath it at the same time?! I thought the current process is the most recent newly created fork(), how can you manipulate? By task switching!!

P1 is the initial process
The fork system call be used by P1 to create new child processes with PIDs ranging from 2 to 8
After creating a child process, either the parent or the child can continue executing and perform additional fork system calls to create more processes
The switch system call allows switching between processes, allowing for arbitrary process hierarchies by executing fork calls from different processes
The exit system calls allow us to terminate any process, except for P1
By carefully coordinating the sequence of fork, switch, and exit system calls, it is possible to construct any desired process tree structure with a maximum of 8 nodes

Run:
`make clean && make`
`./kernel`
