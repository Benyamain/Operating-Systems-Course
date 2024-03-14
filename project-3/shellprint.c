#include "Llibc.h"

#define BUFSIZE 1024    /* Line buffer size */
#define NTOKS 128       /* Max number of tokens in a line */
#define NCMDS 64        /* Max number of full commands in the pipeline */

/*
    Function setptrs():  Given a line buffer, set up an array of
    token pointers as described.

    Return 0 on success, nonzero on error.
*/
int
setptrs(int bufsize, char buf[], char *ptk[])
{
    printf("Inside setptrs function\n");
    int in_token = 0;
    printf("Initialized in_token = %d\n", in_token);
    int tok_idx = 0;
    printf("Initialized tok_idx = %d\n", tok_idx);
    int buf_idx = 0;
    printf("Initialized buf_idx = %d\n", buf_idx);

    ptk[tok_idx] = NULL;
    printf("Set ptk[%d] = NULL\n", tok_idx);

    while (buf_idx < bufsize) {
        printf("Inside while loop, buf_idx = %d\n", buf_idx);
        if (!in_token && buf[buf_idx] != ' ' && buf[buf_idx] != '|' && buf[buf_idx] != '\n') {
            in_token = 1;
            printf("Setting in_token = %d\n", in_token);
            ptk[tok_idx++] = &buf[buf_idx];
            printf("Set ptk[%d] = &buf[%d]\n", tok_idx - 1, buf_idx);
        } else if (in_token && (buf[buf_idx] == ' ' || buf[buf_idx] == '|' || buf[buf_idx] == '\n')) {
            in_token = 0;
            printf("Setting in_token = %d\n", in_token);
            buf[buf_idx] = '\0';
            printf("Set buf[%d] = '\\0'\n", buf_idx);
            if (buf[buf_idx] == '|' || buf[buf_idx] == '\n') {
                ptk[tok_idx++] = NULL;
                printf("Set ptk[%d] = NULL\n", tok_idx - 1);
            }
        }
        buf_idx++;
    }

    ptk[tok_idx] = NULL;
    printf("Set ptk[%d] = NULL (terminating the array)\n", tok_idx);
    return 0;
}

/* The main shell */
int
Lmain(void)
{
    printf("Inside Lmain function\n");
    char buf[BUFSIZE];
    printf("Initialized buf array\n");
    char *ptk[NTOKS];
    printf("Initialized ptk array\n");
    char **pcm[NCMDS];
    printf("Initialized pcm array\n");

    int k, m, r;
    printf("Declared variables k, m, r\n");
    int fdprev[2], fdnext[2];
    printf("Declared arrays fdprev and fdnext\n");

    char *HOMEDIR = "/tmp";
    printf("Set HOMEDIR = \"/tmp\"\n");

    /* The main shell loop */
    while (1 == 1) {
        printf("Inside the main shell loop\n");
        /* Print the command prompt */
        Lprintf("$ ");
        printf("Printed the command prompt\n");

        /* Read a user input line into buffer buf[] */
        if (Lread(0, buf, BUFSIZE) <= 0) {
            printf("Lread returned <= 0, breaking out of the loop\n");
            break;
        }
        printf("User input read into buf\n");

        /* Set up the ptk[] array of pointers for the entire pipeline */
        if ((r = setptrs(BUFSIZE, buf, ptk)) != 0) {
            printf("setptrs returned non-zero value, returning from Lmain\n");
            return r;
        }
        printf("ptk array set up for the pipeline\n");

        /* Set up the array pcm[] of pointers to the pointers
           to the first command words */
        m = 0;
        printf("Initialized m = %d\n", m);
        for (k = 0; !(ptk[k] == 0 && ptk[k + 1] == 0); k++) {
            printf("Inside for loop, k = %d\n", k);
            if (ptk[k] == 0 && ptk[k + 1] != 0) {
                pcm[m++] = &ptk[k + 1];
                printf("Set pcm[%d] = &ptk[%d]\n", m - 1, k + 1);
            }
        }
        printf("pcm array set up, m = %d\n", m);

        /* Implement cd (in the case of a single command vec without pipes) */
        if (m == 1 && Lstrcmp(*pcm[0], "cd") == 0) {
            printf("Single command vector and it's 'cd'\n");
            if (pcm[0][1] == NULL) {
                printf("Changing directory to HOMEDIR = %s\n", HOMEDIR);
                Lchdir(HOMEDIR);
            } else if (pcm[0][2] == NULL) {
                printf("Changing directory to %s\n", pcm[0][1]);
                Lchdir(pcm[0][1]);
            } else {
                Lprintf("cd: too many arguments\n");
                printf("Printed 'cd: too many arguments'\n");
            }
            continue;
        }
        printf("Not a single 'cd' command\n");

        /* Need just two pairs of FDs for the pipes! */
        fdprev[0] = fdprev[1] = fdnext[0] = fdnext[1] = -1;
        printf("Initialized fdprev and fdnext arrays with -1\n");

        for (k = 0; k < m; k++) {    /* Iterate over the commands */
            printf("Inside for loop over commands, k = %d\n", k);
            if (k != 0) {
                /* Not the head command, so the preceding command's
                   write-end pipe will be this command's read-end pipe.
                   Just shift the FDs accordingly!  */
                fdprev[0] = fdnext[0];
                printf("Set fdprev[0] = %d\n", fdprev[0]);
                fdprev[1] = fdnext[1];
                printf("Set fdprev[1] = %d\n", fdprev[1]);
            }

            if (k != m - 1) {     /* Not tail, create a write-end pipe! */
                Lpipe(fdnext);
                printf("Created a new pipe, fdnext = [%d, %d]\n", fdnext[0], fdnext[1]);
            }

            /* Fork child for this command */
            if (Lfork() == 0) {
                printf("Inside child process\n");
                if (k != m - 1) {     /* Not tail */
                    Lclose(fdnext[0]);
                    printf("Closed fdnext[0] = %d\n", fdnext[0]);
                    Ldup2(fdnext[1], 1);
                    printf("Duplicated fdnext[1] = %d to stdout\n", fdnext[1]);
                    Lclose(fdnext[1]);
                    printf("Closed fdnext[1] = %d\n", fdnext[1]);
                }

                if (k != 0) {         /* Not head */
                    Lclose(fdprev[1]);
                    printf("Closed fdprev[1] = %d\n", fdprev[1]);
                    Ldup2(fdprev[0], 0);
                    printf("Duplicated fdprev[0] = %d to stdin\n", fdprev[0]);
                    Lclose(fdprev[0]);
                    printf("Closed fdprev[0] = %d\n", fdprev[0]);
                }

                /* Exec this command! */
                Lexecvp(*pcm[k], pcm[k]);
                int sysret = errno;
                Lprintf("execvp error for %s, errno = %d\n", *pcm[k], sysret);
                printf("Printed execvp error message\n");
                Lexit(1);
            }
            printf("Back in parent process\n");

            /* Back to parent - parent has no use for pipes! */
            if (k > 0) {
                Lclose(fdprev[0]);
                printf("Closed fdprev[0] = %d\n", fdprev[0]);
                Lclose(fdprev[1]);
                printf("Closed fdprev[1] = %d\n", fdprev[1]);
            }
        }
        printf("Done executing commands in the pipeline\n");

        /* Parent is done with setting up and executing the whole pipeline,
           so now wait for all the m children created above */
        for (k = 0; k < m; k++) {
            int status;
            printf("Waiting for child process %d\n", k);
            Lwait(&status);
        }
        printf("All child processes have finished\n");
    }

    Lprintf("\n");
    printf("Exiting Lmain\n");
    return 0;
}
