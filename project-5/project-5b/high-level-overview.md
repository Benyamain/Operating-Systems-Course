graph TD
    A[MT System Initialization] --> B[Create Initial Process P0]
    B --> C[Fork Process P1 from P0]
    C --> D[Task Switch to P1]
    D --> E[User Program Execution]
    E --> F{User Input}
    F --> |'f' - Fork| G[kfork System Call]
    F --> |'a'/'b' - Exec| H[kexec System Call]
    F --> |'s' - Switch| I[do_switch System Call]
    F --> |'q' - Exit| J[do_exit System Call]
    F --> |'z' - Sleep| K[do_sleep System Call]
    F --> |'w' - Wakeup| L[do_wakeup System Call]
    F --> |'t' - Wait| M[do_wait System Call]
    F --> |'v' - View| N[printTaskQueues Function]
    G --> O[savectxfork Assembly Routine]
    O --> P[kfork C Function]
    P --> Q{Free Process Available?}
    Q --> |Yes| R[Initialize Child Process]
    R --> S[Copy Parent's Stack and Context]
    S --> T[Fix Child's Context]
    T --> U[Enqueue Child to readyQueue]
    Q --> |No| V[Error: No Free Process]
    H --> W[Re-initialize Current Process Context]
    W --> X[Transfer Control to New Program]
    I --> Y[tswitch Assembly Routine]
    Y --> Z[scheduler C Function]
    Z --> AA[Enqueue Running Process to readyQueue]
    AA --> AB[Dequeue Next Process from readyQueue]
    J --> AC[kexit C Function]
    AC --> AD[Set Process Status to ZOMBIE]
    AD --> AE[Wake Up Parent Process]
    AE --> AF[Handle Orphaned Processes]
    K --> AG[ksleep C Function]
    AG --> AH[Set Process Status to SLEEPING]
    AH --> AI[Enqueue Process to sleepList]
    L --> AJ[kwakeup C Function]
    AJ --> AK[Wake Up Processes Waiting for Event]
    AK --> AL[Move Processes to readyQueue]
    M --> AM[kwait C Function]
    AM --> AN{Child Process Exited?}
    AN --> |Yes| AO[Retrieve Exit Status]
    AO --> AP[Free Child Process]
    AN --> |No| AQ[Put Parent Process to Sleep]
    N --> AR[Print Running Process Info]
    AR --> AS[Print readyQueue]
    AS --> AT[Print freeList]



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
