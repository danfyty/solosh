#include <runcmd.h>
#include <stdlib.h>

int runcmd(const char* command, int* result, const int* io)
{
	return 0;
}

void (*runcmd_onexit)(void) = NULL;
