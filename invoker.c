#include <stdio.h>
#include <stdlib.h>	// setenv
#include <string.h>	// strncmp, strcpy
#include <unistd.h>	// fork, sleep, execvp
#include <sys/types.h>	// waitpid
#include <sys/wait.h>	//waitpid
#include <errno.h>	// errno
#include <signal.h>	// signal
#include <glob.h>
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
	CMD_TABLE_ROW(jobs),
	{ NULL, NULL }	// Marks end of table.
};

/*	Call built-in cmds or create process for executables,
*	with further args supplied from cmdlArgv.
*	Return value indicates errors. */
Jobs* handleCmdl(Jobs* jobs, char* cmdl, int* cmdlArgc, char ***cmdlArgv, int processCount) {
	/** Returns on reading empty cmd. **/
	/** Searches for built-in cmd strings matched **/
	if (processCount == 1) {
		int i = 0;
		if (cmdlArgc[0] == 0) {
			DEBUG(printf("#DEBUG# Cmd is empty string.\n"););
			return jobs;
		}
		while (cmdProfileTable[i].name != NULL &&
			strcmp((*cmdlArgv)[0], cmdProfileTable[i].name) != 0)
			i++;
			DEBUG(printf("#DEBUG# cmdProfileTable[%d] matched. "
			"[-1] means none matched.\n", i < 4 ? i : -1););
			if (cmdProfileTable[i].name != NULL) {
				/** Call built-in cmd **/
				if ((jobs = cmdProfileTable[i].method(*cmdlArgc, *cmdlArgv, jobs)) < 0) {
					fprintf(stderr, "Error in executing built-in cmd.\n");
					return jobs;
				}
			} else {
				if (((jobs = execProg(jobs, cmdl, cmdlArgv,processCount))) < 0) {
					fprintf(stderr, "Error in executing programme.\n");
					return jobs;
				}
			}
		}
		else {
			/** Execute other programme **/
			if (((jobs = execProg(jobs, cmdl, cmdlArgv,processCount))) < 0) {
				fprintf(stderr, "Error in executing programme.\n");
				return jobs;
			}
		}
		return jobs;
	}

	/*	Create process to execute programme with args, specified in cmdlArgv.
	*	Returns when child process stops. */
	Jobs* execProg(Jobs* jobs, char* cmdl, char ***cmdlArgv, int processCount) {
		setenv("PATH", "/bin:/usr/bin:.", 1);
		int pid;
		pid_t* childPid;
		//int **p = NULL;
		int p[2];
		int oldP[2];
		int fd_in = 0;
		int i = 0,j;
		childPid = calloc((processCount + 1), sizeof(pid_t));
		while (i<processCount) {
			DEBUG(printf("#DEBUG# Creating process for %s.\n", cmdlArgv[i][0]););
			if (i < processCount - 1) {
				pipe(p);
			}
			if ((pid = fork())) {
				DEBUG_ZOMBIE(
					sleep(1);
					printf("#DEBUG# Parent slept. Check for zombies then press enter.");
					getchar();
				);
				childPid[i] = pid;
				setpgid(pid, childPid[0]);
				if (i != processCount - 1){
					close(p[1]);
				}
				if (i != 0) {
					close(oldP[0]);
				}
				oldP[0] = p[0];
				oldP[1] = p[1];
				i++;
				DEBUG_ZOMBIE(
					printf("#DEBUG# Child stopped and parent woke. "
					"Check for zombies then press enter.");
					getchar();
				);
			} else {
				DEBUG(printf("#DEBUG# Child executing %s.\n", cmdlArgv[i][0]););
				/** Set signal handlers to default **/
				if (childPid[0] == 0) {
					childPid[0] = getpid();
				}
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);
				setpgid(0, childPid[0]);
				if(i != processCount - 1) {
					close(p[0]);
					dup2(p[1], 1);
				}
				if(i != 0) {
					close(oldP[1]);
					dup2(oldP[0], 0);
				}
				//wildcard handling
				if (cmdlArgv[i][1] != NULL) {
					glob_t results;
					printf("%s\n",cmdlArgv[i][1]);
					glob(cmdlArgv[i][1], GLOB_NOCHECK, NULL, &results);
					j = 2;
					while (cmdlArgv[i][j] != NULL) {
						printf("%s\n",cmdlArgv[i][j]);
						glob(cmdlArgv[i][j], GLOB_NOCHECK | GLOB_APPEND, NULL, &results);
						j++;
					}
					DEBUG(printf("match %zu\n", results.gl_pathc);
				);
				cmdlArgv[i] = realloc(cmdlArgv[i], sizeof(char *) * (results.gl_pathc + 2));
				for(j = 0;j < results.gl_pathc;j++) {
					cmdlArgv[i][j+1] = malloc(sizeof(char) * strlen(results.gl_pathv[j]));
					strcpy(cmdlArgv[i][j+1], results.gl_pathv[j]);
				}
				cmdlArgv[i][j+1] = NULL;
				globfree(&results);
			}
			DEBUG(
				for (j = 0; cmdlArgv[i][j] != NULL; j++)
				printf("#DEBUG# cmdlArgv[%d]=%s=END\n", j, cmdlArgv[i][j]);
				printf("#DEBUG# cmdlArgv[%d]=%s=END\n", j, cmdlArgv[i][j]);
			);
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
	jobs = waitChildren(childPid, jobs, cmdl);
	return jobs;
}
