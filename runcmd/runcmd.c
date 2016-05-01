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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 20
#define CMD_DELIMITERS " "
#define EXECFLAGFILE ".execfailed~"

int runcmd(const char* command, int* result, const int* io)
{
	char* args[MAX_ARGS], *cmd = NULL, *cur = NULL;
	int nargs = 0, cstatus, fd;
	pid_t cpid;

	cmd = (char*) malloc((strlen(command)+1)*sizeof(char));
	sysfail(cmd == NULL, -1);

	strcpy(cmd, command);
	args[nargs++] = strtok(cmd, CMD_DELIMITERS);
	while (nargs < MAX_ARGS && (cur = strtok(NULL, CMD_DELIMITERS)) != NULL)
	{
		args[nargs++] = cur;
	}

	if (nargs == MAX_ARGS && strtok(NULL, CMD_DELIMITERS) != NULL)
	{
		fprintf(stderr, "Error: too many arguments. Runcmd can't handle more than %d arguments.\n", MAX_ARGS);
		return -1;
	}

	args[nargs] = NULL;

	cpid = fork();
	sysfail(cpid < 0, -1);

	/* Child process code block */
	if (cpid == 0)
	{
		execvp(args[0], args);

		/* Only gets here if exec fails*/
		close(open(EXECFLAGFILE, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR));
		free(cmd);					
		exit(EXECFAILSTATUS);
	}
	/* Child process code block ends here */

	waitpid(cpid, &cstatus, 0);
	if (result != NULL)
	{
		*result = 0;
		if (WIFEXITED(cstatus))
		{
			*result |= WEXITSTATUS(cstatus);
			*result |= NORMTERM;
			if ((fd = open(EXECFLAGFILE, O_RDONLY)) != -1)
			{
				close(fd);
				unlink(EXECFLAGFILE);
			}
			else
			{
				*result |= EXECOK;
			}
		}
	}

	free(cmd);
	return cpid;
}

void (*runcmd_onexit)(void) = NULL;
