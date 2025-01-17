#ifndef __PROCESS_H__
#define __PROCESS_H__

int process_execv(char *path, char *argv[]);
int process_kill(int pid);

#endif