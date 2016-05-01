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
#include <runcmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int ret = 0, i;
	char buffer[1024];

	if (argc < 2)
	{
		printf("Error: no command to execute.\n");
		return 0;
	}

	strcpy(buffer, argv[1]);
	i = 2;
	while (i < argc)
	{
		strcat(buffer, " ");
		strcat(buffer, argv[i++]);
	}

	runcmd(buffer, &ret, NULL);

	printf("EXITSTATUS = %d\nIS_EXECOK = %d\nIS_NORMTERM = %d\n", EXITSTATUS(ret), IS_EXECOK(ret), IS_NORMTERM(ret));
	return 0;
}
