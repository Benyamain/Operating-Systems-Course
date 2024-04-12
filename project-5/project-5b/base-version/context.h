/**
    Offset to context area within PCB struct PROC (see type.h).
    Separated here from type.h for use by the assembler.
 **/
#define OFF_CTX_PCB 40      /* Given PROC *p, this == p - &(p->uctx[0]) */
