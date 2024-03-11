#include <stdio.h>
#include "fs.h"

void
main()
{
	/* create_fs(); */
	/* sync_fs(); */

	create_fs();
	sync_fs();

	/* Test by writing to the filesystem with methods above, then commenting them out and test reading from it */

	/* mount_fs(); */
	/* mount_fs(); */

	/* Runs independently and should never be commented out when testing */
	print_fs();
	printf("Filesystem was initialized and written to. Have a great rest of your day!\n");
}
