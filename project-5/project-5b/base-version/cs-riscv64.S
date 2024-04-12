/*
    cs-riscv64.S : MT context switch routines in riscv64 assembly (GAS)

    This assembly source file implements the context saving and
    restoring routines for fork/exec/tswitch

*/

    /********** riscv64 task CPU context area uctx[] contents **************
    *                                                                      *
    *   The "context" is uctx[], with contents:  |x0|x1|x2|x3|...|x31|     *
    *   (saved CPU registers).  Each uctx[k] is a 64bit long (8 bytes).    *
    *                                                                      *
    *   uctx[] contents:     uctx[0]   uctx[1]   uctx[2]         uctx[31]  *
    *                      | x0 = 0 | x1 = ra | x2 = sp | . . . |  x31  |  *
    *                        zero      retPC    stk-ptr                    *
    *   uctx byte offset:      0         8        16      . . .    248     *
    *   PROC byte offset:     40        48        56      . . .    288     *
    *                                                                      *
    ***********************************************************************/

/* For the macro OFF_CTX_PCB:  Offset of uctx[] within struct PROC */
#include "context.h"

/* Use a uniform stack pointer delta (max common value) */
#define SPDELTA     16

    /* External global variables/functions that this file uses */
    .global running, kfork, scheduler

    /* Functions defined here that need to be exported for use elsewhere */
    .global savectxfork, tswitch, runctx


    /***********************************************************************
    *
    *   int savectxfork(void);
    *
    * Snapshot CPU registers of the `running' process into its context
    * (for duplication later), call kfork(), and return to caller
    *
    * Steps:
    *   - Immediately freeze/save all live CPU registers to running->uctx[]
    *     by appropriately calling the local routine savecallerctx
    *   - Call kfork() to create a child duplicating the just-saved context
    *   - Return to caller
    *
    * Notes:
    *   - We save CPU regs into context via another routine savecallerctx,
    *     but calling it will modify ra (x1), so we have to save it, which
    *     in turn will requre modifying sp (x2) too.  But the savecallerctx
    *     routine has been coordinated in a way that it can correctly obtain
    *     our original x1 and x2 values and save them into context!
    *   - When calling a register saving routine, avoid `call' pseudo instr;
    *     the assembler may translate it in a way that clobbers x6/x5.
    *     Use `jal' instead, which is a fully determined real instruction.
    ************************************************************************/
    savectxfork:
        addi    sp, sp, -SPDELTA    /* We are modifiying sp by -SPDELTA */
        sd      x1, 0(sp)           /* Save x1 (ra), calls will clobber it */
        jal     savecallerctx       /* jal == near call, but a real instr */
        call    kfork               /* Create a cloned child */
        /* a0 now = return code from kfork(), 0 for child, pid for parent */
        ld      x1, 0(sp)
        addi    sp, sp, SPDELTA
        ret
    /********************** End of savectxfork() ***************************/


    /***********************************************************************
    *
    *       int tswitch(void);      // Does not return to original caller!
    *
    * Task-switch the `running' process via scheduler()
    *
    * Steps:
    *   - Immediately freeze/save all live CPU registers to running->uctx[]
    *     by calling savecallerctx appropriately
    *   - Call scheduler() to pick a new value for `running' (PCB ptr switch)
    *   - Resume from the context inside new running->uctx[], transferring
    *      PC control to the saved ra address in running->uctx[1]
    *
    *   When called from one process this "function" returns to another one!
    ************************************************************************/
    tswitch:
        addi    sp, sp, -SPDELTA    /* We are modifying sp by -SPDELTA */
        sd      x1, 0(sp)           /* Save x1 (ra), calls will clobber it */
        jal     savecallerctx       /* jal == near call */
        call    scheduler           /* Change ptr running: pick a new PROC */
        call    runctx              /* Or use no-return j (jump) */
        /* NO RETURN HERE! */
    /************************ End of tswitch() ****************************/


    /***********************************************************************
    *
    *   int runctx(void);       // Does not return - transfers control!
    *
    * Start/restart execution from context pointed to by the `running'
    * PCB pointer ("run from running->uctx[]")
    *
    * Start executing from PC address saved in running->uctx[1],
    * after loading all live CPU registers (including stack ptr)
    * from running->uctx[].
    *
    * Steps:
    *   - Load all live CPU register from the saved ones in running->uctx[]
    *   - Jump to the saved return PC addr found there
    *
    * So the ret at the and of this function does not return to caller,
    * but starts executation from whatever address was saved in uctx[1].
    *
    * This function can be used to start/restart any program with
    * PC/execution address saved in uctx[1], with its registers and
    * stack initialized from the values found in uctx[].  E.g., here
    * it is invoked after calling scheduler() to resume a previously
    * switched-out process.
    ************************************************************************/
    runctx:          /* run/resume from stored context */
        /* x31 is a ptr to context area; reload it at the end from uctx[31] */
        ld      x31, running
        addi    x31, x31, OFF_CTX_PCB   /* Now x31 points to running->uctx */
        ld      x1, 8(x31)              /* Restore x1, set ra = uctx[1] */
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
 
        ld      x31, 248(x31)       /* Load x31 itself! Must be done last! */
 
        ret                         /* Resume: Control goes to saved ra addr!
                                       Won't return to caller, as the ra value
                                       was changed:  The orginal ra value from
                                       caller was overwritten with the one
                                       found saved in context uctx[1]. */
    /************************ End of runctx() *****************************/


    /***********************************************************************
    *
    *   void savecallerctx(void);
    *
    * Save caller's "original registers" to context area running->uctx[],
    * i.e., the registers the caller had at its own entry are saved
    * via this called function!
    *
    * This assumes the following understanding at our entry:
    *
    *   - Caller's original x1 (ra) to be saved is now in 0(current_sp)
    *     (as the caller had to save it on stack before calling us)
    *   - Caller's original x2 (sp) is == current_sp + SPDELTA
    *     (as the caller decremented sp by SPDELTA before calling us)
    *   - All other cur live registers are same as caller's original values
    *
    * At return, we restore all registers to what they were at our entry.
    *
    * Registers we will modify and restore:  sp (x2), x30, x31.
    * We don't touch ra (== x1).
    *
    * Notes:
    *   - This is a static (private local) LEAF function.
    *   - Normally context saving of live register must be done
    *     without calling any auxiliary helper function since the
    *     call iself will need modification of x1 (ra) and x2 (sp).
    *     So here we handle those two registers specially, using
    *     agreed upon locations/changes to restore the original values.
    *   - This function should be called using `jal', since `call'
    *     may be expanded to a form that clobbers x6 or x5.
    ************************************************************************/
    savecallerctx:
        /* We use x31 as a ptr, and x30 as a tmp */
        addi    sp, sp, -SPDELTA        /* current_sp == orig_sp - 2*SPDELTA */
        sd      x30, 0(sp)              /* Save x30, to be used as a tmp */
        sd      x31, 8(sp)              /* Save x31, to be used as a ptr */

        /* Make x31 point to running->uctx[0] (NOT `la x31, running'!) */
        ld      x31, running            /* x31 = *running, not &running[0] */
        addi    x31, x31, OFF_CTX_PCB   /* x31 now points to the context */

        /* Save original x1 */
        ld      x30, 0+SPDELTA(sp)      /* Get original x1 into tmp, and */
        sd      x30, 8(x31)             /* ... save it into uctx[1] */

        /* Save original x2 (sp) */
        mv      x30, x2                 /* Let tmp = current sp */
        addi    x30, x30, 2*SPDELTA     /* As cur_sp == orig_x2 - 2*SPDELTA */
        sd      x30, 16(x31)            /* Save original x2 into uctx[2] */

        /* Save original x3-x29 */
        sd      x3, 24(x31)
        sd      x4, 32(x31)
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

        /* Save original x30 */
        ld      x30, 0(sp)              /* Get original x30 into tmp, and */
        sd      x30, 240(x31)           /* ... save it into uctx[30] */

        /* Save original x31 */
        ld      x30, 8(sp)              /* Get original x31 into tmp, and */
        sd      x30, 248(x31)           /* ... save it into uctx[31] */

        /* Restore registers we have modified (x31, x30, sp) */
        ld      x31, 8(sp)
        ld      x30, 0(sp)
        addi    sp, sp, SPDELTA
        ret                             /* Return to caller */

    /********************* End of savecallerctx() *************************/


/*************************************************************************

************************************
*** Description of savectxfork() ***
************************************

At entry to savectxfork(), the stack is essentially the same as that of
the parent process which made the do_kfork() syscall, and the register
ra (== x1) holds the return address from the caller, where the caller
is the do_kfork() function.  So ra holds the address of do_kfork()'s
next line/instruction where it made the savectxfork() call (== address
of the next instruction after do_kfork() that would have been executed
y the calling process).

savectxfork() then performs the following sequence of actions:

- Save all registers of running parent process into its PCB context area

- Call kfork() to create a new child process.
  kfork() does the following:
      - Copy all context and all stack from parent to child
      - In the child's context, update the stack pointer so that it
        points to the corresponding position in the child's stack
        (not the parent's stack), and update the function return value
        register a0 to the value 0 (when scheduled to run, the child should
        see a return value of 0 from do_fork() syscall made by the parent)
      - Enqueue the child into readyQue and return with the PID of the child

********************************
*** Description of tswitch() ***
********************************

At entry to tswitch(), the stack is essentially the same as that of the
running/outgoing process which made the do_switch() syscall, and the
register ra (== x1) holds the return address from the caller, where
the caller is the do_switch() function.  So ra holds the address of
doswitch()'s next line/instruction where it made the tswitch() call
(== address of the next instruction that would have been executed
by the calling process after do_switch()).

tswitch() then performs the following sequence of actions:

- Save all registers of outgoing running process into its PCB context area
  (This is called "Conext Switch Out", which "freezes" the current/outgoing
  process to its exact state when tswitch() was entered)

- Call scheduler() to pick a new (incoming) process to resume/run.
  scheduler() changes the "running" ptr from old PROC to new PROC, i.e.:
      - At entry "running" points to the old/outgoing process PCB
      - At return "running" points to the new/incoming process PCB

- (Reseume) Run the runctx "function", which will:
    - Restore all registers from the context of PROC pointed to by running;
      in particular, the stack now is the same as what it was for the
      incoming PROC when it got switched out
    - Resume the incoming PROC to start running again from where it was
      switched out.  Since ra has been restored from the incoming PROC's
      context, the last ret instruction of runctx transfers execution
      over to this address in ra, instead of returning to runctx's caller
  (This is called "Context Switch In", after which the stack and registers
  are the same as they were when the incoming process was switched out)

Reference:  K. C. Wang, Systems Programming in Unix/Linux, Springer 2018.

**************************************************************************/

