/*
  C runtime startup code (crt) for calling main() in riscv64/linux

  Abhijit Dasgupta (MIT license), 2024 February 10, February 24

  Linux exec vector ABI:  At start of linux execve we have:

    sp          -->     argc
    sp + 1      -->     argv[0]
    sp + 2      -->     argv[1]
    ...,
    sp + argc-1 -->     argv[n-1]
    sp + argc   -->     (null)
    sp + argc+1 -->     envp[0]

  So per riscv64 calling ABI, need to call main after putting:

    a0 = argc   (== [sp])
    a1 = argv   (== &argv[0] == &argc + 1 == sp + 8)
    a2 = envp   (== &envp[0] == argv + argc + 1 == sp + 8 + 8*[sp] + 8)

  Also save these in the following (storage allocated in Llibc.c):
    int __Largc;
    char ** __Largv;
    char ** __Lenvp;

*/

/* From /usr/include/asm-generic/unistd.h (or #include it) */
#define SYS_exit 93

                    /* .global errno is stored in syscall.S */

                    /* Assembler directives */
                    .text
                    .globl  _start      /* Linker default entry point */
                    .globl  Lmain
                    .globl  __Largc
                    .globl  __Largv
                    .globl  __Lenvp
                    .align  8

/* Default linker symbol for program entry point */
_start:
                    mv fp, sp               /* set up first stack frame */

                    ld a0, 0(sp)            /* a0 has argc */
                    mv a1, sp
                    addi a1, a1, 8          /* now a1 = &argv[0] */

                    /* Search for terminating null ptr at end of argv[] */
                    mv t1, a1
countargvloop:
                    mv t0, zero
                    ld t0, 0(t1)
                    beq t0, zero, done      /* found null ptr argv[argc] */
                    addi t1, t1, 8
                    j countargvloop
done:
                    addi t1, t1, 8
                    mv a2, t1               /* now a2 = &envp[0] */

                    /* Save argc (a0) into Llibc variable __Largc */
                    la      t1, __Largc
                    sw      a0, 0(t1)
                    /* Save argv (a1) into Llibc variable __Largv */
                    la      t1, __Largv
                    sd      a1, 0(t1)
                    /* Save envp (a2) into Llibc variable __Lenvp */
                    la      t1, __Lenvp
                    sd      a2, 0(t1)

                    call Lmain

_exitloop:
                    /* mv a1, a0 */         /* a0 has return code from main */
                    li a7, SYS_exit         /* Syscall number for _exit() */
                    ecall
                    j _exitloop             /* Should not happen! */

