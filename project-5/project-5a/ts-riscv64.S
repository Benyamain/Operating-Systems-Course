/*
    ts-riscv64.S :  Task switcher (dispatcher) code in GAS riscv64 assembly

  This assembly source file defines the tswitch() function, implementing
  the do_switch() system call used by user programs.  See comments below.

*/

/* For the macro OFF_CTX_PCB:  Offset of uctx[] within struct PROC */
#include "context.h"

    /* Functions and global variables exported/defined by this file:
        int tswitch(void);            // this function (described below)
        int resumerunningctx(void);   // resume from running->uctx[] and run
    */
    .global tswitch, resumerunningctx

    /* External functions and global variables used by this file:
        PROC *running;          // ptr to PCB of current "running" process
        int scheduler(void);    // enqueue "running" and pick a new one
    */
    .global running, scheduler

    .text
    /* Use gcc/as -fno-pic for non-PIC/non-GOT PC-relative relocation! */

tswitch:
SAVE:
    /*  SAVE part:
        On entry: ra (== x1) contains return addr from calling process
        Goal:  Save all live CPU registers of outgoing process into its
               context area running->uctx[] inside PCB (struct PROC)
    */

    /******* riscv64 task context area uctx[] and stack contents ********
    
    The "context" is uctx[], with contents:  |x0|x1|x2|x3|...|x31|
    (saved CPU registers).  Each uctx[k] is a 64bit long (8 bytes).

    uctx[] contents:     uctx[0]   uctx[1]   uctx[2]         uctx[31]
                       | x0 = 0 | x1 = ra | x2 = sp | . . . |  x31   |
                         zero      retPC    stk-ptr
    uctx byte offset:      0         8        16      . . .    248
    PROC byte offset:     40        48        56      . . .    288

    ********************************************************************/

    /* Save all live registers into current process context running->uctx[].
       The saved register values must be exactly what they are now at entry! */

    /** We will use x31 as a ptr to the current context running->uctx[],
        so we must first save its orig value on stack for restoring later */
    addi    sp, sp, -16             /* Make room in stack (avoid red zone) */
    sd      x31, 0(sp)              /* Save x31, for use as ptr to uctx[] */

    ld      x31, running            /* x31 = *running, so x31 now points to
                                       the PCB of current/outgoing process */
    addi    x31, x31, OFF_CTX_PCB   /* Advance x31 to point to context area,
                                       i.e. now x31 == &(running->uctx[0]) */

    /** x31 is now a pointer to running->uctx[] **/

    /** We have modified x2 (== sp) and x31, and will save them in uctx[]
        later, after appropriate adjustments.  But we can immediately
        save all the other registers:  x1 and x3-x30 **/

    sd      x1, 8(x31)              /* Save x1 (== ra) into uctx[1] */
                                    /* SKIP x2, it will be saved later */
    sd      x3, 24(x31)             /* Save x3 into uctx[3] */
    sd      x4, 32(x31)             /* Save x4 into uctx[4] */
    sd      x5, 40(x31)
    sd      x6, 48(x31)
    sd      x7, 56(x31)
    sd      x8, 64(x31)
    sd      x9, 72(x31)
    sd      x10, 80(x31)
    sd      x11, 88(x31)
    sd      x12, 96(x31)
    sd      x13, 104(x31)
    sd      x14, 112(x31)
    sd      x15, 120(x31)
    sd      x16, 128(x31)
    sd      x17, 136(x31)
    sd      x18, 144(x31)
    sd      x19, 152(x31)
    sd      x20, 160(x31)
    sd      x21, 168(x31)
    sd      x22, 176(x31)
    sd      x23, 184(x31)
    sd      x24, 192(x31)
    sd      x25, 200(x31)
    sd      x26, 208(x31)
    sd      x27, 216(x31)
    sd      x28, 224(x31)
    sd      x29, 232(x31)
    sd      x30, 240(x31)           /* Save x30 into uctx[30] */
                                    /* SKIP x31, it will be saved later */

    /* We need to get the original values of the clobbered x2, x31.
       Since x30 has already been saved, we can use it as a tmp. */

    /* Save original x31 into context area, using x30 as tmp */
    ld      x30, 0(sp)              /* Get original x31 from stack into x30 */
    sd      x30, 248(x31)           /* Save original x31 into uctx[31] */

    /* The orig value of x2 (== sp) was decremented by 16, so we adjust. */
    addi    sp, sp, 16              /* x2 == sp now restored to orig_val */
    sd      x2, 16(x31)             /* Original x2 == sp saved to uctx[2] */

    /* All original registers saved into context.  But live x30 and x31
       remain clobbered, so we restore them too (optional/unneeded). */
    ld      x30, 240(x31)           /* Restore x30 from saved context */
    ld      x31, 248(x31)           /* Restore x31 itself! Must be done last! */

    /* At this point entire CPU context is saved AND all live registers
       are back to the values they had when tswitch/SAVE was entered */

FIND:
    call    scheduler        /* change ptr running: pick a new running PROC */

RESUME:
/* Resume and run a program whose context is in running->uctx[] */
resumerunningctx:
/*  int resumerunningctx(void);  // NO RETURN!

    Loads all live CPU register from the saved ones in running->uctx[]
    (saved context), then jumps PC to the saved return addr found there.

    So the ret at the and of this function does not return to its caller,
    but starts executation from whatever address was saved in uctx[1].

    This function will ususally run after scheduler (as in here), but it
    can also be used to start any program whose address is saved in uctx[1],
    with registers and stack initialized to the values from uctx[].
*/

    /* Use x31 as ptr to context area - it will be restored later */
    ld      x31, running
    addi    x31, x31, OFF_CTX_PCB   /* Now x31 points to saved context area */

    ld      x1, 8(x31)              /* Restore x1 : ra = uctx[1] == saved_ra */
    ld      x2, 16(x31)             /* Restore x2 */
    ld      x3, 24(x31)             /* Restore x3 */
    ld      x4, 32(x31)
    ld      x5, 40(x31)
    ld      x6, 48(x31)
    ld      x7, 56(x31)
    ld      x8, 64(x31)
    ld      x9, 72(x31)
    ld      x10, 80(x31)
    ld      x11, 88(x31)
    ld      x12, 96(x31)
    ld      x13, 104(x31)
    ld      x14, 112(x31)
    ld      x15, 120(x31)
    ld      x16, 128(x31)
    ld      x17, 136(x31)
    ld      x18, 144(x31)
    ld      x19, 152(x31)
    ld      x20, 160(x31)
    ld      x21, 168(x31)
    ld      x22, 176(x31)
    ld      x23, 184(x31)
    ld      x24, 192(x31)
    ld      x25, 200(x31)
    ld      x26, 208(x31)
    ld      x27, 216(x31)
    ld      x28, 224(x31)
    ld      x29, 232(x31)
    ld      x30, 240(x31)

    ld      x31, 248(x31)       /* Restore x31 itself! Must be done last! */

    ret                         /* Resume: Control goes to saved ra addr!
                                   Won't return to caller, as the ra value
                                   was changed:  The orginal ra value from
                                   caller was overwritten with the one
                                   found saved in context uctx[1]. */
/*
Description of the full tswitch() code:

At entry to tswitch(), the stack is essentially the same as that of the
running/outgoing process which made the do_switch() syscall, and the
register ra (== x1) holds the return address from the caller, where
the caller is the do_switch() function.  So ra holds the address of
doswitch()'s next line/instruction where it made the tswitch() call
(== address of the next instruction that would have been executed
by the calling process after do_switch()).

tswitch() then performs the following sequence of actions:

(1) SAVE all registers of outgoing running process into its PCB context area

(2) FIND: Call scheduler() to pick a new (incoming) process to resume/run:
    - Scheduler() changes the "running" ptr from old PROC to new PROC, i.e.:
      - At entry "running" points to the old/outgoing process PCB
      - At return "running" points to the new/incoming process PCB

(3) RESUME: Run the resumerunningctx "function", which will:
    - Restore all registers from the context of PROC pointed to by running;
      in particular, the stack now is the same as what it was for the
      incoming PROC when it got switched out
    - Resume the incoming PROC to start running again from where it was
      switched out (the ret instruction does this:  Since ra has been
      restored from the the incoming PROC context, resumerunningctx
      does not return to its caller!)

Step (1) is known as "Conext Switch Out".  It saves ("freezes") the
current/outgoing process to its exact state when tswitch() was entered.

Step (3) is called "Context Switch In", after which the stack and
registers are the same as they were when the incoming process was
switched out, and (with the last `ret') this "function" resumes
execution of the (incoming) process that was picked by the scheduler.

Reference:  K. C. Wang, Systems Programming in Unix/Linux, Springer 2018.

*/

