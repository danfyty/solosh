#include <runcmd.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	char* cmd = "./t1 127";
	int ret = 0;
	runcmd(cmd, &ret, NULL);
	printf("EXITSTATUS = %d\nIS_EXECOK = %d\nIS_NORMTERM = %d\n", EXITSTATUS(ret), IS_EXECOK(ret), IS_NORMTERM(ret));
	return 0;
}
