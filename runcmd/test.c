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
