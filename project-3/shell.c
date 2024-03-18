#include "Llibc.h"

#define BUFSIZE 1024    /* Line buffer size */
#define NTOKS 128       /* Max number of tokens in a line */
#define NCMDS 64        /* Max number of full commands in the pipeline */

int redirection_in(const char* file_path);
int redirection_out(const char* file_path, int append_mode);

/*
    Function tokenize_buffer():  Given a line buffer, set up an array of
    token pointers.

    Return 0 on success, nonzero on error.
*/
int tokenize_buffer(int buffer_size, char buffer[], char *token_ptrs[]) {
    int input_tkn = 0, tok_idx = 0, buf_idx;
    char prev_buf_char = '\0';

    token_ptrs[tok_idx] = NULL;

    for (buf_idx = 0; buf_idx < buffer_size && buffer[buf_idx] != '\n'; buf_idx++) {
        if (buffer[buf_idx] == ' ' || buffer[buf_idx] == '\t' || buffer[buf_idx] == '|' || buffer[buf_idx] == '\n') {
           if (input_tkn) {
              buffer[buf_idx] = '\0';
              input_tkn = 0;
           }

           if (buffer[buf_idx] == '|' && prev_buf_char != '\0') {
              token_ptrs[++tok_idx] = NULL;
           }
        }
        else {
	   if (!input_tkn) {
               token_ptrs[++tok_idx] = &buffer[buf_idx];
               input_tkn = 1;
           }
        }
        prev_buf_char = buffer[buf_idx];
    }

    if (input_tkn) {
       buffer[buf_idx] = '\0';
    }

    token_ptrs[++tok_idx] = NULL;
    token_ptrs[++tok_idx] = NULL;

    return 0;
}

int redirection_in(const char* file_path) {
    int fd = Lopen(file_path, O_RDONLY);

    if (fd < 0) {
       return -1;
    }

    if (Ldup2(fd, 0) < 0) {
       return -1;
    }

    Lclose(fd);
    return 0;
}

int redirection_out(const char* file_path, int append_mode) {
    int flags;

    if (append_mode) {
       flags = O_WRONLY | O_CREAT | O_APPEND;
    }
    else {
       flags = O_WRONLY | O_CREAT | O_TRUNC;
    }

    int fd = Lopen(file_path, flags, 0666);

    if (fd < 0) {
       return -1;
    }

    if (Ldup2(fd, 1) < 0) {
       return -1;
    }

    Lclose(fd);
    return 0;
}

int Lmain(void) {
    char buffer[BUFSIZE];
    char *token_ptrs[NTOKS];
    char **cmd_ptrs[NCMDS];

    int num_cmds, r;
    int prev_pipe[2], next_pipe[2];

    char *HOME_DIR = "/tmp";

    while (1 == 1) {
        Lprintf("$ ");

        if (Lread(0, buffer, BUFSIZE) <= 0) {
            break;
        }

        if ((r = tokenize_buffer(BUFSIZE, buffer, token_ptrs)) != 0) {
            return r;
        }

        num_cmds = 0;
        for (int d = 0; !(token_ptrs[d] == 0 && token_ptrs[d + 1] == 0); d++) {
            if (token_ptrs[d] == 0) {
                cmd_ptrs[num_cmds++] = &token_ptrs[d + 1];
            }
        }

        if (num_cmds == 0) {
            continue;
        }

        if (num_cmds == 1 && Lstrcmp(cmd_ptrs[0][0], "cd") == 0) {
            char *dir;
            if (cmd_ptrs[0][1]) {
               dir = cmd_ptrs[0][1];
            }
            else {
               dir = HOME_DIR;
            }
            if (Lchdir(dir) != 0) {
                Lprintf("Could not cd to \"%s\"\n", dir);
            }
            continue;
        }

        prev_pipe[0] = prev_pipe[1] = next_pipe[0] = next_pipe[1] = -1;

        for (int k = 0; k < num_cmds; k++) {
	    if (k != 0) {
	       prev_pipe[0] = next_pipe[0];
               prev_pipe[1] = next_pipe[1];
	    }

            if (k != num_cmds - 1) {
                Lpipe(next_pipe);
            }

            int child_pid = Lfork();
            if (child_pid == 0) {
                if (k != num_cmds - 1) {
                    Ldup2(next_pipe[1], 1);
		    Lclose(next_pipe[0]);
                    Lclose(next_pipe[1]);
                }

                if (k != 0) {
                    Ldup2(prev_pipe[0], 0);
                    Lclose(prev_pipe[0]);
		    Lclose(prev_pipe[1]);
                }

                for (int i = 0; cmd_ptrs[k][i] != NULL; i++) {
                    if (Lstrcmp(cmd_ptrs[k][i], "<") == 0) {
                        if (redirection_in(cmd_ptrs[k][i + 1]) < 0) {
			   Lexit(1);
			}
                        cmd_ptrs[k][i] = NULL;
                        break;
                    } else if (Lstrcmp(cmd_ptrs[k][i], ">") == 0) {
                        if (redirection_out(cmd_ptrs[k][i + 1], 0) < 0) {
			   Lexit(1);
			}
                        cmd_ptrs[k][i] = NULL;
                        break;
                    } else if (Lstrcmp(cmd_ptrs[k][i], ">>") == 0) {
                        if (redirection_out(cmd_ptrs[k][i + 1], 1) < 0) {
			   Lexit(1);
			}
                        cmd_ptrs[k][i] = NULL;
                        break;
                    }
                }

                Lexecvp(cmd_ptrs[k][0], cmd_ptrs[k]);
                int sysret = errno;
                Lprintf("execvp error for %s, errno = %d\n", *cmd_ptrs[k], sysret);
                Lexit(1);
            }
	    else if (child_pid < 0) {
                Lfprintf(2, "Fork failed, errno = %d\n", errno);
            }

            if (k > 0) {
                Lclose(prev_pipe[0]);
                Lclose(prev_pipe[1]);
            }
        }

        for (int p = 0; p < 2; p++) {
            if (prev_pipe[p] != -1) {
	       Lclose(prev_pipe[p]);
	    }
            if (next_pipe[p] != -1) {
	       Lclose(next_pipe[p]);
	    }
        }

        for (int c = 0; c < num_cmds; c++) {
           int status;
	   Lwait(&status);
        }
    }

    return 0;
}
