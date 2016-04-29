#ifndef RUNCMD_H
#define RUNCMD_H

#define EXECFAILSTATUS 127
#define EXITSTATUSBYTE 0xFF

#define NORMTERM (1 << 8)
#define EXECOK (1 << 9)

#define EXITSTATUS(ret) ((ret & EXITSTATUSBYTE))
#define IS_NORMTERM(ret) ((ret & NORMTERM) && 1)
#define IS_EXECOK(ret) ((ret & EXECOK) && 1)
#define IS_NONBLOCK

int runcmd(const char *command, int *result, const int* io);
extern void (*runcmd_onexit)(void);

#endif
