#include <stdio.h>
#include <stdlib.h>	// setenv
#include <string.h>	// strncmp, strcpy
#include <unistd.h>	// fork, sleep, execvp
#include <sys/types.h>	// waitpid
#include <sys/wait.h>	//waitpid
#include <errno.h>	// errno
#include <signal.h>	// signal
#include "invoker.h"
#include "debug.h"

/*	Table of built-in cmd strings and method pointers.
*	Refer to invoker.h for definition of CmdProfile.
*	Method names cmdMain must agree with builtInCmds.h. */
#define CMD_TABLE_ROW(cmd) { #cmd, cmd##Main }
// E.g. CMD_TABLE_ROW(cd) becomes { "cd", cdMain },

CmdProfile cmdProfileTable[] = {
	CMD_TABLE_ROW(cd),
	CMD_TABLE_ROW(exit),
	CMD_TABLE_ROW(fg),
	CMD_TABLE_ROW(job),
	{ NULL, NULL }	// Marks end of table.
};

/*	Call built-in cmds or create process for executables,
*	with further args supplied from cmdlArgv.
*	Return value indicates errors. */
int handleCmdl(int* cmdlArgc, char ***cmdlArgv, int processCount) {
	/** Returns on reading empty cmd. **/
	/** Searches for built-in cmd strings matched **/
	if (processCount == 1) {
		int i = 0;
		if (cmdlArgc[0] == 0) {
			DEBUG(printf("#DEBUG# Cmd is empty string.\n"););
			return 0;
		}
		while (cmdProfileTable[i].name != NULL &&
			strcmp((*cmdlArgv)[0], cmdProfileTable[i].name) != 0)
			i++;
			DEBUG(printf("#DEBUG# cmdProfileTable[%d] matched. "
			"[-1] means none matched.\n", i < 4 ? i : -1););
			if (cmdProfileTable[i].name != NULL) {
				/** Call built-in cmd **/
				if (cmdProfileTable[i].method(*cmdlArgc, *cmdlArgv) < 0) {
					fprintf(stderr, "Error in executing built-in cmd.\n");
					return -1;
				}
			} else {
				if (execProg(cmdlArgv,processCount) < 0) {
					fprintf(stderr, "Error in executing programme.\n");
					return -2;
				}
			}
		}
		else {
			/** Execute other programme **/
			if (execProg(cmdlArgv,processCount) < 0) {
				fprintf(stderr, "Error in executing programme.\n");
				return -2;
			}
		}
		return 0;
	}

	/*	Create process to execute programme with args, specified in cmdlArgv.
	*	Returns when child process stops. */
	int execProg(char ***cmdlArgv, int processCount) {
		setenv("PATH", "/bin:/usr/bin:.", 1);
		int childStatus;
		int pid;
		int childPid[processCount];
		int p[2];
		int fd_in = 0;
		int i = 0;
		while (i<processCount) {
			DEBUG(printf("#DEBUG# Creating process for %s.\n", cmdlArgv[i][0]););
			pipe(p);
			if ((pid = fork())) {
				DEBUG_ZOMBIE(
					sleep(1);
					printf("#DEBUG# Parent slept. Check for zombies then press enter.");
					getchar();
				);
				childPid[i] = pid;
				close(p[1]);
				fd_in = p[0];
				i++;
				DEBUG_ZOMBIE(
					printf("#DEBUG# Child stopped and parent woke. "
					"Check for zombies then press enter.");
					getchar();
				);
			} else {
				DEBUG(printf("#DEBUG# Child executing %s.\n", cmdlArgv[i][0]););
				/** Set signal handlers to default **/
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				if (fd_in != 0) {
					dup2(fd_in, 0);
				}
				if (i < processCount -1) {
					dup2(p[1], 1);
				}
				close(p[0]);
				/** Execute programme **/
				if (execvp(cmdlArgv[i][0], cmdlArgv[i]) == -1) {
					if (errno == ENOENT)
					printf("%s:  command not found\n", cmdlArgv[i][0]);
					else
					printf("%s:  unknown error\n", cmdlArgv[i][0]);
				}
				exit(0);
			}
		}
		for(i = 0; i < processCount; i++) {
			waitpid(childPid[i], &childStatus, WUNTRACED);
			DEBUG(
				if (WIFSIGNALED(childStatus))
				printf("#DEBUG# Child was terminated by signal %d.\n",
				WTERMSIG(childStatus));
			);
		}
		return 0;
	}
