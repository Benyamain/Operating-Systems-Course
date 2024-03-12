#include "Llibc.h"

int
Lmain(void)
{
    int decisecs = 1234;
    Lprintf("Sleeping %d deciseconds ...\n", decisecs);
    Lsleep(decisecs);
    return 0;
}
