#include <stdio.h>
#include "fs.h"

void
main()
{
	create_fs();
	sync_fs();
	printf("Filesystem was initialized and written to. Have a great rest of your day!\n");
}
