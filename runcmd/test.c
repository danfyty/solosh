/*   test.c - a simple libruncmd testing program
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
#include <fcntl.h>
#include <runcmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE_BUFFER_SIZE 1024

void onexit()
{
	printf("\nruncmd_exit() has just ran.\n");	
}

int main(int argc, char* argv[])
{
	int result = 0, ret, io[3], len, done = 0;
	char buffer[LINE_BUFFER_SIZE];
	
	if (argc != 1 && argc != 4)
	{
		printf("Invalid number of arguments. Must be zero or 3.\n");
		printf("Usage:\n\t./test\n\t./test [INPUT FILE] [OUTPUT FILE] [ERROR FILE]\n");
		return 0;
	}

	if (argc == 4)
	{
		io[0] = open(argv[1], O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
		io[1] = open(argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
		io[2] = open(argv[3], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	}

	runcmd_onexit = onexit;

	
	while(!done)
	{
		while(fgets(buffer, LINE_BUFFER_SIZE, stdin), !(len = strlen(buffer)));		/* The 'while' is necessary. SIGCHLD may unblock the program while it waits for input */
		if (buffer[len-1] == '\n')
			buffer[len-1] = '\0';
		if (!strcmp("exit", buffer))
			done = 1;
		else
		{
			if (argc == 1)
				ret = runcmd(buffer, &result, NULL);	
			else
				ret = runcmd(buffer, &result, io);
		
			printf("***\n");
			printf("Command: \"%s\"\nEXITSTATUS = %d\nIS_EXECOK = %d\nIS_NORMTERM = %d\nIS_NONBLOCK = %d\n", buffer, EXITSTATUS(result), IS_EXECOK(result), IS_NORMTERM(result), IS_NONBLOCK(result));
			printf("runcmd returned %d.\n", ret);
			printf("***\n");
		}
		buffer[0] = '\0';	/* Guarantees - together with the while loop above - that old input won't be reused */
	}
	return 0;
}
