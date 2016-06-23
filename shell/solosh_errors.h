#ifndef SOLOSH_ERROS_H
#define SOLOSH_ERRORS_H

#include <errno.h>
#include <stdio.h>

#define error(expr, code) \
	do\
	{\
		if (expr)\
		{\
			printf("Error: %s\n", strerror(errno));\
			return code;\
		}\
	} while (0)

#define fatal_error(expr, status) \
	do\
	{\
		if (expr)\
		{\
			printf("Fatal error: %s\n", strerror(errno));\
			exit(status);\
		}\
	} while (0)

#endif
