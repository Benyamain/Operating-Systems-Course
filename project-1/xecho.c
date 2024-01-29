#include <unistd.h>

int
lenstr(char *str);

void
printstr(char *str);

void
printarg(int arg, char str[])
{
    int index = 0;
    while (index >= 0) {
        // Check arg to be a single digit num or else
        if (arg >= 0 && arg <= 9) {
                str[index] = (char) 0x30 + (arg % 10);
                index++;
                break;
        }
        else {
                // Get the first digit of the two digit (example) num
                str[index] = (char) 0x30 + ((arg - arg % 10) / 10);
                index++;

                // Next arr index position gets the right most digit
                str[index] = (char) 0x30 + arg % 10;
                index++;
                break;
        }
    }

    str[index] = '\0';

    write(STDOUT_FILENO, str, lenstr(str));
}

void
printquotedstr(char *str)
{
    printstr("\"");

    // Loop through the stored arr of char and handle special char
    for (int index = 0; str[index] != 0x00; index++) {
        if (str[index] < ' ' || str[index] > '~') {
            printstr("\\");
            printarg(str[index], str);
        } else if (str[index] == '\\') {
            printstr("\\\\");
        } else if (str[index] == '\'') {
            printstr("\\'");
        } else {
            write(STDOUT_FILENO, &str[index], 1);
        }
    }

    printstr("\"");
}

void
printarray(char *str)
{
    printstr("{");

    for (int index = 0; str[index] != 0x00; index++) {
        if (index > 0) {
            printstr(", ");
        }

        // Handle special characters
        if (str[index] < 0x20 || str[index] > 0x7E) {
            // Assuming for int to be 4 bytes + the NULL terminator
            char numcode[5];
            int len = 0;

            // Convert numerical code to string manually
            int temp = str[index];
            while (temp > 0) {
                numcode[len++] = (temp % 10) + 0x30;
                temp /= 10;
            }

            // Print special character representation
            write(STDOUT_FILENO, "{", 1);
            for (int i = len - 1; i >= 0; i--) {
                write(STDOUT_FILENO, &numcode[i], 1);
                if (i > 0) {
                    write(STDOUT_FILENO, ", ", lenstr(", "));
                }
            }
            write(STDOUT_FILENO, ", '\\0'}", 8);
        } else if (str[index] == '\t') {
            // Handle tab character
            write(STDOUT_FILENO, "'\\t'", lenstr("'\\t'"));
        } else if (str[index] == '\n') {
            // Handle newline character
            write(STDOUT_FILENO, "'\\n'", lenstr("'\\n'"));
        } else if (str[index] == '\r') {
            // Handle return character
            write(STDOUT_FILENO, "'\\r'", lenstr("'\\r'"));
        } else if (str[index] == '\\') {
            // Handle backslash character
            write(STDOUT_FILENO, "'\\\\'", lenstr("'\\\\'"));
        } else if (str[index] == '\'') {
            // Handle single quote character
            write(STDOUT_FILENO, "'\\''", lenstr("'\\''"));
        } else {
            // Handle regular characters
            write(STDOUT_FILENO, "'", 1);
            write(STDOUT_FILENO, &str[index], 1);
            write(STDOUT_FILENO, "'", 1);
        }
    }

        // Checking for len of str to account for empty sets
    if (str[lenstr(str) - 1] != 0x00) {
        printstr(", '\\0'};\n");
    }
    else {
        printstr("'\\0'};\n");
    }
}

int
main(int argc, char *argv[], char *envp[])
{
    char arg[100];

    // Argc
    printstr("argc == ");
    printarg(argc, arg);
    printstr("\n");

    // Loop through to show all input arg
    for (int index = 0; index < argc; index++) {
        if (index > 0) {
            printstr("argv[");
            printarg(index, arg);
            printstr("] : ");
        } else {
            printstr("argv[");
            printarg(index, arg);
            printstr("] : ");
        }

        // After showing index, output what content is in that index pos
        printquotedstr(argv[index]);
        printstr(" == ");
        printarray(argv[index]);
    }

    // Incrementing
    int envc = 0;
    while (envp[envc] != NULL) {
        envc++;
    }

    // Envc
    printstr("envc == ");
    printarg(envc, arg);
    printstr("\n");

    // Hardcode first envp index
    printstr("envp[0] : ");
    printquotedstr(envp[0]);
    printstr("\n");

    // Show last index in envp
    printstr("...\n");
    printstr("envp[");
    printarg(--envc, arg);
    printstr("] : ");
    printquotedstr(envp[envc]);
    printstr("\n");

    _exit(0);
}
