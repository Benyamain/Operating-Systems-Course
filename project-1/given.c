/*************************
 * FILE given.c          *
 ************************/

#include <unistd.h>         /* No other #include allowed! */

int
lenstr(char *str)           /* Like strlen(3), str must be null-terminated */
{
    int ret;
    for (ret = 0; str[ret] != 0; ret++)
        ;
    return ret;
}

void
printstr(char *str)         /* Functionality of printf("%s", str) */
{
    int len = lenstr(str);
    write(STDOUT_FILENO, str, len);
}

