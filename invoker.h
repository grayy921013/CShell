#include "builtInCmds.h"
/*	Contains the name used in cmd line and the function pointer
 *	to the built-in cmd. */
typedef struct {
	char *const name;
	Jobs* (*const method)(int cmdlArgc, char **cmdlArgv, Jobs* jobs);
} CmdProfile;



Jobs* handleCmdl(Jobs* jobs, char*cmdl, int* cmdlArgc, char ***cmdlArgv, int processCount);

Jobs* execProg(Jobs* jobs, char* cmdl, char ***cmdlArgv, int processCount);
