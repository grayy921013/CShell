#include "builtInCmds.h"

/*	Contains the name used in cmd line and the function pointer
 *	to the built-in cmd. */
typedef struct {
	char *const name;
	int (*const method)(int cmdlArgc, char **cmdlArgv);
} CmdProfile;

int handleCmdl(int* cmdlArgc, char ***cmdlArgv, int processCount);

int execProg(char ***cmdlArgv, int processCount);
