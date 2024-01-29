/**********************
 *  FILE baseconv.c   *
 **********************/

#include <unistd.h>        /* No other #include allowed! */

int
strtonum(int base, char *bufsrc, unsigned int *ndst);

int
numtostr(int base, unsigned int *nsrc, char *bufdst);

int
lenstr(char *str);

void
printstr(char *str);

/* Add lenstr() and printstr() declarations if you are using them (given.c) */

int
main(int argc, char *argv[])
{
        // Check for valid num of args
        if (argc < 4) {
                write(STDERR_FILENO, "Error:  Usage:  baseconv fromBase toBase num1 [num2 ...]\n", lenstr("Error:  Usage:  baseconv fromBase toBase num1 [num2 ...]\n"));
                _exit(10);
        }

        unsigned int frombase = 0, tobase = 0;

        // Convert
        char *frombasestr = argv[1];
        while (*frombasestr != 0x00) {
                frombasestr++;

                int res = strtonum(10, argv[1], &frombase);

                // Error checking
                // Outside base range
                if (res == -1) {
                    write(STDERR_FILENO, "Error:  Invalid base specified\n", lenstr("Error:  Invalid base specified\n"));
                    _exit(9);
                }

                // Invalid digit char
                if (res == -2) {
                    write(STDERR_FILENO, "Error:  Invalid digit character encountered\n", lenstr("Error:  Invalid digit character encountered\n"));
                    _exit(8);
                }

                // Overflow
                if (res == -3) {
                    write(STDERR_FILENO, "Error:  Number too large to be converted\n", lenstr("Error:  Number too large to be converted\n"));
                    _exit(7);
                }
        }

        // Convert
        char *tobasestr = argv[2];
        while (*tobasestr != 0x00) {
                tobasestr++;

                int res = strtonum(10, argv[2], &tobase);

                // Error checking
                // Outside base range
                if (res == -1) {
                    write(STDERR_FILENO, "Error:  Invalid base specified\n", lenstr("Error:  Invalid base specified\n"));
                    _exit(6);
                }

                // Invalid digit char
                if (res == -2) {
                    write(STDERR_FILENO, "Error:  Invalid digit character encountered\n", lenstr("Error:  Invalid digit character encountered\n"));
                    _exit(5);
                }

                // Overflow
                if (res == -3) {
                    write(STDERR_FILENO, "Error:  Number too large to be converted\n", lenstr("Error:  Number too large to be converted\n"));
                    _exit(4);
                }
        }

        // Checking validity of baseconv args
        for (int f = 3; f < argc; f++) {
                unsigned int n;
                int err = strtonum(frombase, argv[f], &n);

                // Error checking
                // Outside base range
                if (err == -1) {
                    write(STDERR_FILENO, "Error:  Invalid base specified\n", lenstr("Error:  Invalid base specified\n"));
                    _exit(3);
                }

                // Invalid digit char
                if (err == -2) {
                    write(STDERR_FILENO, "Error:  Invalid digit character encountered\n", lenstr("Error:  Invalid digit character encountered\n"));
                    _exit(2);
                }

                // Overflow
                if (err == -3) {
                    write(STDERR_FILENO, "Error:  Number too large to be converted\n", lenstr("Error:  Number too large to be converted\n"));
                    _exit(1);
                }

                char res[100];
                int reslen = numtostr(tobase, &n, res);

                // Output result if every condition is passed
                write(STDOUT_FILENO, res, reslen);
                printstr(" ");
        }
}
