/*************************
 *                       *
 *  FILE functions.c     *
 *                       *
 *************************/

/* This file must be pure standalone C code, containing only the
   four C functions below.  In particular:
   - It must not include any headers!
   - It must not use any library function or POSIX call at all!
   - Functions here cannot call any function outside of this file
     (but they are allowed to call each other if necessary)
*/


/* Function 1:

   Convert a single-digit numeric int to its char value (max base 16)

     char idtoc(int d) { }

   - Takes an integer input
   - Requires the input to be between 0 and 15 (inclusive)
   - Returns the character for that int digit, assuming the maximum
     possible base of 16 (that is, hex)
*/

char
idtoc(int d)
{
        // Mapping hex
        char hexachars[] = "0123456789abcdef";

        // Check range
        if (d >= 0 && d <= 15) {
                return hexachars[d];
        }
        else {
                // ERR should be char that does not exist within range, other functions check it
                return 0x00;
        }
}


/* Function 2:

   Convert a single digit char in a given base to its int value

     int ctoid(int base, char c) { }

   - This is a kind of an inverse function of idtoc()
   - The first input parameter is an int specifying the conversion base
     which is required to be between 2 and 16 (inclusive)
   - The second parameter is any character (for use by this function)
   - Return value:  If the character c represents a valid digit
     in the given base, returns the int value of that digit, else
     returns -1.  For bases between 11 and 16 (inclusive), allow
     both upper case and lower case characters as digits.
*/

int
ctoid(int base, char c)
{
        char hexachars[] = "0123456789ABCDEF";
        int index = -1;

        // Check base range
        if (base >= 2 && base <= 16) {
                if (c >= 0x61 && c <= 0x66) {
                    // Make them uppercase
                    c -= 0x20;
                }
                for (int d = 0; d < base; d++) {
                        if (hexachars[d] == c) {
                                // Found char match, exit the loop
                                index = d;
                                break;
                        }
                }
                return index;
        }
        else {
                // ERR
                return -1;
        }
}


/* Function 3:

   Convert a char digit-string to the int it represents (in the given base)

     int strtonum(int base, char *bufsrc, unsigned int *ndst) { }

   - The first input parameter is an int specifying the conversion base
     which is required to be between 2 and 16 (inclusive)
   - The second input parameter is a pointer to a character string,
     which is required to be null-terminated string
   - The third input parameter is a pointer to an unsigned int
     (for storing the converted integer)
   - Return value:  If all inputs are valid and the conversion can be
     done successfully, then return 0.  Otherwise return a negative
     integer whose value is informative, for example:
        Return -1
            if the base is not a valid value (between 2 and 16);
        Return -2
            if the character string pointed to by buf contains an invalid
                char (i.e. any char other than the digit characters for
                the given base);
        Return -3
            if the string is a valid digit string in the given base
                but the integer it represents is too large to be stored
                as an unsigned int (overflow).  Note that an int
                is a 32-bit quantity, so an unsigned int's max value is
                (unsigned int) ((int) -1) == 0xffffffff == 4294967295.
   - As usual, the caller is responsible for valid storage at the
     locations pointed to by all the pointer parameters that are passed
*/

int
strtonum(int base, char *bufsrc, unsigned int *ndst)
{
        // Check base range
        if (base >= 2 && base <= 16) {
                unsigned int res = 0;
                while (*bufsrc != 0x00) {
                        int d;

                        // Return the digit
                        d = ctoid(base, *bufsrc);

                        // Char conversion causes int output to be incorrect
                        if (d < 0) {
                            return -2;
                        }

                        // Check for overflow
                        if (res > (4294967295 - d) / base) {
                                return -3;
                        }

                        res = res * base + d;
                        bufsrc++;
                }
                *ndst = res;
                return 0;
        }
        else {
                // ERR
                return -1;
        }
}


/* Function 4:

   Convert an int to its char-digit-string representation (in the given base)

     int numtostr(int base, unsigned int *nsrc, char *bufdst) { }

   - The first input parameter is an int specifying the conversion base
     which is required to be between 2 and 16 (inclusive)
   - The second input parameter is a pointer to the unsigned int
     (required to be non-negative) that needs to be converted to its
     representation as a string of digit characters in the given base
   - The third parameter is a pointer to (the start of) a buffer of chars;
     the caller is required to have allocated sufficient storage
     to this buffer for holding the converted character digit string
     including the terminating null character
   - Return value:  On success, return the length of the computed
     digit string (without leading zero), and return -1 on any error
*/

int
numtostr(int base, unsigned int *nsrc, char *bufdst)
{

        // Check valid base range
        if (base >= 2 && base <= 16) {
                // // A case of the input num to be NULL
                // if (*nsrc == 0) {
                //         bufdst[0] = 0x30;
                //         bufdst[1] = 0x00;
                //         return 1;
                // }

                // Redirector var
                unsigned int g = *nsrc;
                int len = 0;

                // Loop through to find num, store into char arr, place conditions on num
                while (g > 0) {
                        int d = g % base;
                        bufdst[len++] = idtoc(d);
                        // Double digit num
                        g /= base;
                }

                // Tail is at the end of char arr, decrement for arr index notation
                int head = 0, tail = --len;

                // Loop through the arr in reverse order
                while (head < tail) {
                        char t = bufdst[head];
                        bufdst[head] = bufdst[tail];
                        bufdst[tail] = t;
                        ++head;
                        --tail;
                }

                // Last index becomes the NULL terminator
                bufdst[++len] = 0x00;
                return len;
        }
        else {
                return -1;
        }
}

/*******************************
 *  END OF FILE functions.c    *
 *******************************/
