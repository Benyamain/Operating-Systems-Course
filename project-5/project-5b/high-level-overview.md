MT System Initialization
|
+-- Create Initial Process P0
+-- Fork Process P1 from P0
+-- Task Switch to P1
+-- User Program Execution
    +-- User Input
        +-- 'f' - Fork
        |   +-- kfork System Call
        |       +-- savectxfork Assembly Routine
        |       |   +-- Save current process context
        |       |   +-- Call kfork C Function
        |       +-- kfork C Function
        |           +-- Check for free process slot in freeList
        |           +-- Free Process Available?
        |           |   +-- Yes
        |           |   |   +-- Initialize Child Process
        |           |   |   +-- Copy Parent's Stack and Context
        |           |   |   +-- Fix Child's Context
        |           |   |   +-- Enqueue Child to readyQueue
        |           |   +-- No
        |           |       +-- Error: No Free Process
        |           +-- Error: No Free Process
        +-- 'a'/'b' - Exec
        |   +-- kexec System Call
        |       +-- Re-initialize Current Process Context
        |       +-- Transfer Control to New Program
        +-- 's' - Switch
        |   +-- do_switch System Call
        |       +-- tswitch Assembly Routine
        |       |   +-- Save current process context
        |       |   +-- Call scheduler C Function
        |       +-- scheduler C Function
        |       |   +-- Enqueue Running Process to readyQueue
        |       |   +-- Dequeue Next Process from readyQueue
        |       +-- Enqueue Running Process to readyQueue
        |       +-- Dequeue Next Process from readyQueue
        +-- 'q' - Exit
        |   +-- do_exit System Call
        |       +-- kexit C Function
        |           +-- Set Process Status to ZOMBIE
        |           +-- Add Process to zombieList
        |           +-- Wake Up Parent Process
        |           |   +-- Remove Parent from sleepList
        |           +-- Handle Orphaned Processes
        +-- 'z' - Sleep
        |   +-- do_sleep System Call
        |       +-- ksleep C Function
        |           +-- Set Process Status to SLEEPING
        |           +-- Enqueue Process to sleepList
        +-- 'w' - Wakeup
        |   +-- do_wakeup System Call
        |       +-- kwakeup C Function
        |           +-- Wake Up Processes Waiting for Event
        |           +-- Remove Processes from sleepList
        |           +-- Move Processes to readyQueue
        +-- ^tb^ - Wait
        |   +-- do_wait System Call
        |       +-- kwait C Function
        |           +-- Check for Zombie Child Processes
        |           +-- Child Process Exited?
        |           |   +-- Yes
        |           |   |   +-- Retrieve Exit Status
        |           |   |   +-- Remove Child from zombieList
        |           |   |   +-- Add Child PCB to freeList
        |           |   +-- No
        |           |       +-- Is Parent Process P1?
        |           |       |   +-- Yes
        |           |       |   |   +-- Check for Zombie Child Processes
        |           |       |   |   |   +-- Found
        |           |       |   |   |   |   +-- Retrieve Exit Status
        |           |       |   |   |   +-- Not Found
        |           |       |   |   |       +-- Continue P1 Execution
        |           |       |   +-- No
        |           |       |       +-- Put Parent Process to Sleep
        |           +-- Put Parent Process to Sleep
        +-- 'v' - View
            +-- printTaskQueues Function
                +-- Print Running Process Info
                +-- Print readyQueue
                +-- Print freeList
                +-- Print sleepList
                +-- Print zombieList



1. MT System Initialization:
	* The system starts by creating the initial process P0.
	* P0 forks process P1, and a task switch occurs to start executing P1.

2. User Program Execution:
	* The user program (uprogA or uprogB) runs and prompts for user input.
	* Depending on the user input, different system calls or actions are triggered.

3. Process Creation (Fork):
	* When the user enters 'f', the kfork system call is invoked.
	* The savectxfork assembly routine saves the current process context.
	* The kfork C function checks for a free process slot in the freeList.
	* If a free process is available, the child process is initialized by copying the parent's stack and context.
	* The child's context is fixed, and the child process is enqueued to the readyQueue.

4. Program Execution (Exec):
	* When the user enters 'a' or 'b', the kexec system call is invoked.
	* The current process context is re-initialized, and control is transferred to the new program.

5. Process Switching:
	* When the user enters 's', the do_switch system call is invoked.
	* The tswitch assembly routine saves the current process context and calls the scheduler C function.
	* The scheduler function enqueues the running process to the readyQueue and dequeues the next process to run.

6. Process Termination (Exit):
	* When the user enters 'q', the do_exit system call is invoked.
	* The kexit C function sets the process status to ZOMBIE and wakes up the parent process.
	* Orphaned processes are handled by making P1 their new parent.

7. Process Synchronization (Sleep/Wakeup):
	* When the user enters 'z', the do_sleep system call is invoked, calling the ksleep C function.
	* The process status is set to SLEEPING, and the process is enqueued to the sleepList.
	* When the user enters 'w', the do_wakeup system call is invoked, calling the kwakeup C function.
	* Processes waiting for a specific event are woken up and moved to the readyQueue.

8. Process Synchronization (Wait):
	* When the user enters 't', the do_wait system call is invoked, calling the kwait C function.
	* If a child process has exited (status is ZOMBIE), its exit status is retrieved, and the child process is freed.
	* If no child process has exited, the parent process is put to sleep until a child exits.

9. Viewing System State:
	* When the user enters 'v', the printTaskQueues function is called.
	* It prints information about the currently running process, the readyQueue, and the freeList.
