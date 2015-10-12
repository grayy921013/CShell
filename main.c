/*
 *	Author: Ivan Lin, 1155017469 Vincent Zhou, 1155014433
 *	Project: CSCI3150 Shell Phase 2
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
	char tempCmdl[INPUT_MAX];
	char ***cmdlArgv;
	int **cmdlArgc ;
	int processCount;
	Jobs* jobs = NULL;
	/** Set signal handlers to ignore **/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	cmdlArgc = malloc(sizeof(int*));
	/** Main loop **/
	while (1) {
		cmdLinePrompt(cmdl);
		strcpy(tempCmdl, cmdl);
		cmdlArgv = tokenizeCmdl(cmdl, cmdlArgc, &processCount);
		jobs = handleCmdl(jobs, tempCmdl, *cmdlArgc, cmdlArgv, processCount);
		freeCmdlArgv(cmdlArgv, processCount);
	}
	return 0;
}
