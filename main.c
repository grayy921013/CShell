/*
 *	Author: Ivan Lin, 1155017469
 *	Project: CSCI3150 Shell Phase 1
 *	Date: Oct 2015
 */

#include <stdio.h>
#include <signal.h>	// signal
#include "shell.h"
#include "invoker.h"
#include "debug.h"

int main(void) {
	DEBUG(printf("#DEBUG# Debug mode.\n"););
	/** Declare variables **/
	char cmdl[INPUT_MAX];
	char ***cmdlArgv;
	int **cmdlArgc ;
	int processCount;
	/** Set signal handlers to ignore **/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	cmdlArgc = malloc(sizeof(int*));
	/** Main loop **/
	while (1) {
		cmdLinePrompt(cmdl);
		cmdlArgv = tokenizeCmdl(cmdl, cmdlArgc, &processCount);
		handleCmdl(*cmdlArgc, cmdlArgv, processCount);
		freeCmdlArgv(cmdlArgv, processCount);
	}
	return 0;
}
