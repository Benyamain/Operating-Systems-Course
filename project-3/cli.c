#include "Llibc.h"

#define BUFSIZE 1024    /* Line buffer size */
#define NTOKS 128       /* Max number of tokens in a line */
#define NCMDS 64        /* Max number of full commands in the pipeline */

/*
	Function setptrs():  Given a line buffer, set up an array of token pointers as described.

	Return 0 on success, nonzero on error.
*/
int
setptrs(int bufsize, char buf[], char *ptk[])
{
	int in_token = 0;
    	int token_idx = 0;
    	int buf_idx = 0;

	// Initialize ptk[0] to NULL
	ptk[0] = NULL;

	// Scan the line byte by byte
	while (buf_idx < bufsize && buf[buf_idx] != '\n')
	{
		char c = buf[buf_idx];

        	if (c == ' ' || c == '|')
		{
            		// End of token
            		if (in_token)
			{
                		buf[buf_idx] = '\0'; // Null the byte after the token
                		in_token = 0;
                		ptk[++token_idx] = NULL; // Add null pointer to mark end of command vector
            		}
        	}
		else
		{
            		// Beginning of token
            		if (!in_token)
			{
                		in_token = 1;
                		ptk[token_idx + 1] = &buf[buf_idx]; // Add pointer to the beginning of the token
            		}
        	}

		buf_idx++;
    	}

    	// Handle the last token
    	if (in_token)
	{
        	buf[buf_idx] = '\0'; // Null the byte after the token
        	ptk[++token_idx] = NULL; // Add null pointer to mark end of command vector
    	}

    	// Add the final NULL pointer to terminate the whole pipeline
    	ptk[++token_idx] = NULL;

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
    	while (1 == 1)
	{

        	/* Print the command prompt */
        	Lprintf("$ ");

        	/* Read a user input line into buffer buf[] */
        	if (Lread(0, buf, BUFSIZE) <= 0)
		{
            		break;
		}

        	/* Set up the ptk[] array of pointers for the entire pipeline */
        	if ((r = setptrs(BUFSIZE, buf, ptk)) != 0)
		{
            		return r;
		}

        	/* Set up the array pcm[] of pointers to the pointers
           	to the first command words */
        	m = 0;
        	for (k = 0; !(ptk[k] == 0 && ptk[k + 1] == 0); k++)
		{
            		if (ptk[k] == 0 && ptk[k + 1] != 0)
			{
                		/* The beginning of a command vector found */
                		pcm[m++] = &ptk[k + 1];
			}
        	}
        	/* Now pcm[0], pcm[1], ..., pcm[m-1] point to the command vectors! */

	        /*
			- If m == 0, then there is no command to run (the user entered an empty line with only whitespace)
	              	- If m == 1, there is exactly one command vector, no pipes
	              	- If m > 1, this is a pipeline!
	        */

        	/* Implement cd (in the case of a single command vec without pipes) */
        	if (m == 1 && Lstrcmp(*pcm[0], "cd") == 0)
		{
            		if (pcm[0][1] == NULL)
			{
                		// cd without any additional params, change CWD to HOMEDIR
                		if (Lchdir(HOMEDIR) != 0)
				{
                    			Lprintf("Could not cd to \"%s\"\n", HOMEDIR);
                		}
            		}
			else if (pcm[0][2] == NULL)
			{
                		// cd with exactly one param, try to change CWD to that directory
                		if (Lchdir(pcm[0][1]) != 0)
				{
                    			Lprintf("Could not cd to \"%s\"\n", pcm[0][1]);
                		}
            		}
			else
			{
                		// cd with more than one param, report an error
                		Lprintf("cd: too many arguments\n");
            		}
            		continue;
		}

        	/* Need just two pairs of FDs for the pipes! */
        	fdprev[0] = fdprev[1] = fdnext[0] = fdnext[1] = -1;

        	/* Iterate over the commands */
		for (k = 0; k < m; k++)
		{

            		if (k != 0)
			{
                		/* Not the head command, so the preceding command's
                   		write-end pipe will be this command's read-end pipe.
                   		Just shift the FDs accordingly!  */
                		fdprev[0] = fdnext[0];
                		fdprev[1] = fdnext[1];
            		}

            		/* Not tail, create a write-end pipe! */
			if (k != m - 1)
			{
                		Lpipe(fdnext);
			}

            		/* Fork child for this command */
            		if (Lfork() == 0)
			{

                		/* Not tail */
				if (k != m - 1)
				{
                    			/* Redirect stdout to the write end of the pipe */
                    			Lclose(fdnext[0]); // Close the read end
                    			Ldup2(fdnext[1], 1); // Redirect stdout to the write end
                    			Lclose(fdnext[1]); // Close the write end copy
                		}

                		/* Not head */
				if (k != 0)
				{
                    			/* Redirect stdin to the read end of the pipe */
                    			Lclose(fdprev[1]); // Close the write end
                    			Ldup2(fdprev[0], 0); // Redirect stdin to the read end
                    			Lclose(fdprev[0]); // Close the read end copy
                		}

                		/* Exec this command! */
                		Lexecvp(*pcm[k], pcm[k]);
                		int sysret = errno;
                		Lprintf("execvp error for %s, errno = %d\n", *pcm[k], sysret);
                		Lexit(1);
            		}

            		/* Back to parent - parent has no use for pipes! */
            		if (k > 0)
			{
                		Lclose(fdprev[0]);
                		Lclose(fdprev[1]);
            		}

        	}

        	/* Parent is done with setting up and executing the whole pipeline,
           	so now wait for all the m children created above */
        	for (k = 0; k < m; k++)
		{
            		int status;
            		Lwait(&status);
        	}
	}

    	Lprintf("\n");
	return 0;
}
