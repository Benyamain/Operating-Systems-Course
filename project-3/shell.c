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
    int in_token = 0;
    int tok_idx = 0;
    int buf_idx = 0;

    ptk[tok_idx] = NULL;

    while (buf_idx < bufsize) {
        if (!in_token && buf[buf_idx] != ' ' && buf[buf_idx] != '|' && buf[buf_idx] != '\n') {
            in_token = 1;
            ptk[tok_idx++] = &buf[buf_idx];
        } else if (in_token && (buf[buf_idx] == ' ' || buf[buf_idx] == '|' || buf[buf_idx] == '\n')) {
            in_token = 0;
            buf[buf_idx] = '\0';
            if (buf[buf_idx] == '|' || buf[buf_idx] == '\n') {
                ptk[tok_idx++] = NULL;
            }
        }
        buf_idx++;
    }

    ptk[tok_idx] = NULL;
    return 0;
}

/* The main shell */
int
Lmain(void)
{
    char buf[BUFSIZE];
    char *ptk[NTOKS];
    char **pcm[NCMDS];

    int k, m, r;
    int fdprev[2], fdnext[2];

    char *HOMEDIR = "/tmp";

    /* The main shell loop */
    while (1 == 1) {
        /* Print the command prompt */
        Lprintf("$ ");

        /* Read a user input line into buffer buf[] */
        if (Lread(0, buf, BUFSIZE) <= 0)
            break;

        /* Set up the ptk[] array of pointers for the entire pipeline */
        if ((r = setptrs(BUFSIZE, buf, ptk)) != 0)
            return r;

        /* Set up the array pcm[] of pointers to the pointers
           to the first command words */
        m = 0;
        for (k = 0; !(ptk[k] == 0 && ptk[k + 1] == 0); k++) {
            if (ptk[k] == 0 && ptk[k + 1] != 0)
                pcm[m++] = &ptk[k + 1];
        }

        /* Implement cd (in the case of a single command vec without pipes) */
        if (m == 1 && Lstrcmp(*pcm[0], "cd") == 0) {
            if (pcm[0][1] == NULL) {
                Lchdir(HOMEDIR);
            } else if (pcm[0][2] == NULL) {
                Lchdir(pcm[0][1]);
            } else {
                Lprintf("cd: too many arguments\n");
            }
            continue;
        }

        /* Need just two pairs of FDs for the pipes! */
        fdprev[0] = fdprev[1] = fdnext[0] = fdnext[1] = -1;

        for (k = 0; k < m; k++) {    /* Iterate over the commands */
            if (k != 0) {
                /* Not the head command, so the preceding command's
                   write-end pipe will be this command's read-end pipe.
                   Just shift the FDs accordingly!  */
                fdprev[0] = fdnext[0];
                fdprev[1] = fdnext[1];
            }

            if (k != m - 1) {     /* Not tail, create a write-end pipe! */
                Lpipe(fdnext);
            }

            /* Fork child for this command */
            if (Lfork() == 0) {
                if (k != m - 1) {     /* Not tail */
                    Lclose(fdnext[0]);
                    Ldup2(fdnext[1], 1);
                    Lclose(fdnext[1]);
                }

                if (k != 0) {         /* Not head */
                    Lclose(fdprev[1]);
                    Ldup2(fdprev[0], 0);
                    Lclose(fdprev[0]);
                }

                /* Exec this command! */
                Lexecvp(*pcm[k], pcm[k]);
                int sysret = errno;
                Lprintf("execvp error for %s, errno = %d\n", *pcm[k], sysret);
                Lexit(1);
            }

            /* Back to parent - parent has no use for pipes! */
            if (k > 0) {
                Lclose(fdprev[0]);
                Lclose(fdprev[1]);
            }
        }

        /* Parent is done with setting up and executing the whole pipeline,
           so now wait for all the m children created above */
        for (k = 0; k < m; k++) {
            int status;
            Lwait(&status);
        }
    }

    Lprintf("\n");
    return 0;
}
