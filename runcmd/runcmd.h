#ifndef RUNCMD_H
#define RUNCMD_H

int runcmd(const char *command, int *result, const int* io);
extern void (*runcmd_onexit)(void);

#endif
