/*   runcmd.c - run a command as a subprocess
     Copyright (C) 2016 Rodrigo Weigert <rodrigo.weigert@usp.br>

     This file is part of SoloSH.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <debug.h>
#include <fcntl.h>
#include <runcmd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 20
#define CMD_DELIMITERS " "
#define MAX_PROCESS_PID_VALUE (1 << 15)

/* If you know of a solution to the "persistent non-blocking mode" problem that doesn't involve 
using global variables, feel free to let me know. I don't. */
static int nonblock_flag[MAX_PROCESS_PID_VALUE];

void child_term_handler(int sig, siginfo_t* info, void *u);

int runcmd(const char* command, int* result, const int* io)
{
	char* args[MAX_ARGS], *cmd = NULL, *cur = NULL;
	int nargs = 0, cstatus, execfailpipe[2], noblock = 0;
	pid_t cpid;
	struct sigaction act;

	/* Parsing command */

	cmd = (char*) malloc((strlen(command)+1)*sizeof(char));
	sysfail(cmd == NULL, -1);

	strcpy(cmd, command);
	args[nargs++] = strtok(cmd, CMD_DELIMITERS);

	while (nargs < MAX_ARGS && (cur = strtok(NULL, CMD_DELIMITERS)) != NULL)
		args[nargs++] = cur;

	if (nargs == MAX_ARGS && strtok(NULL, CMD_DELIMITERS) != NULL)
	{
		fprintf(stderr, "Error: too many arguments. Runcmd can't handle more than %d arguments.\n", MAX_ARGS);
		return -1;
	}

	args[nargs] = NULL;
	noblock = args[nargs-1][0] == '&';
	if (noblock)
		args[--nargs] = NULL;

	/* Registering SIGCHLD handler */

	memset(&act, 0, sizeof(struct sigaction));
	act.sa_flags |= SA_SIGINFO;
	act.sa_sigaction = child_term_handler;
	sysfail(sigaction(SIGCHLD, &act, NULL) < 0, -1);

	/* Creating pipe used to detect exec failure. Forking.*/

	sysfail(pipe(execfailpipe) < 0, -1);
	fcntl(execfailpipe[0], F_SETFD, FD_CLOEXEC);
	fcntl(execfailpipe[1], F_SETFD, FD_CLOEXEC);

	cpid = fork();
	sysfail(cpid < 0, -1);

	/* Child process code block */
	if (cpid == 0)
	{
		if (io != NULL)
		{
			int i;
			for (i = 0; i < 3; i++)
			{
				if (io[i] != i)
				{
					close(i);
					dup(io[i]);
					/*close(io[i]);*/
				}
			}
		}
		execvp(args[0], args);

		/* Only gets here if exec fails*/
		write(execfailpipe[1], "@", 1);
		close(execfailpipe[0]);
		close(execfailpipe[1]);
		free(cmd);
		exit(EXECFAILSTATUS);
	}
	/* Child process code block ends here */

	close(execfailpipe[1]);
	if (noblock)
	{
		if (result != NULL)
			*result = NONBLOCK;
		nonblock_flag[cpid] = 1;
	}
	else
	{
		waitpid(cpid, &cstatus, 0);
		if (result != NULL)
		{
			*result = 0;
			if (WIFEXITED(cstatus))
			{
				*result |= WEXITSTATUS(cstatus);
				*result |= NORMTERM;
				if (read(execfailpipe[0], NULL, 1) == 0)
					*result |= EXECOK;
			}	
		}
	}
	close(execfailpipe[0]);
	free(cmd);
	return cpid;
}	

void child_term_handler(int sig, siginfo_t* info, void *u)
{
	int status = 0;
	pid_t pid = info->si_pid;

	waitpid(pid, &status, 0);
	if (nonblock_flag[pid])
	{
		nonblock_flag[pid] = 0;
		if (runcmd_onexit != NULL)
			runcmd_onexit();
	}
}

void (*runcmd_onexit)(void) = NULL;
