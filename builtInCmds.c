#include <stdio.h>
#include <stdlib.h>	// exit
#include <unistd.h>	// chdir
#include <sys/types.h>
#include <signal.h>
#include "builtInCmds.h"
#include "debug.h"

/**	Function names, arguments and return values must follow the convension. **/

/*	Changes current working directory. Accepts only exactly 1 arg. */
Jobs* cdMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs) {
	DEBUG(printf("#DEBUG# Calling cd.\n"););
	if (cmdlArgc != 2) {
		printf("%s:  wrong number of arguments\n", cmdlArgv[0]);
		//return 1;
	}
	if (chdir(cmdlArgv[1]) == -1) {
		printf("%s:  cannot change directory\n", cmdlArgv[1]);
		//return 2;
	}
	return jobs;
}

/*	Exit from the programme. Accepts only 0 args. */
Jobs* exitMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs) {
	DEBUG(printf("#DEBUG# Calling exit.\n"););
	if (cmdlArgc != 1) {
		printf("%s:  wrong number of arguments\n", cmdlArgv[0]);
		return jobs;
	}
	exit(0);
}

Jobs* fgMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs) {
	int index;
	DEBUG(printf("#DEBUG# Calling fg.\n"););
	if (cmdlArgc != 2) {
		printf("%s:  wrong number of arguments\n", cmdlArgv[0]);
		return jobs;
	}
	index = atoi(cmdlArgv[1]);
	if (index > 0) {
		jobs = wakeJob(jobs, index);
	} else {
		printf("fg: no such job\n");
	}
	return jobs;
}

Jobs* jobsMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs) {
	Jobs* temp = jobs;
	int count = 0;
	DEBUG(printf("#DEBUG# Calling jobs.\n"););
	if (cmdlArgc != 1) {
		printf("%s:  wrong number of arguments\n", cmdlArgv[0]);
		return jobs;
	}
	while (temp) {
		printf("[%d] %s\n", ++count, temp->cmd);
		temp = temp->next;
	}
	if (count == 0) {
		printf("No suspended jobs\n");
	}
	return jobs;
}
Jobs* wakeJob(Jobs* jobs, int index) {
	int i = 0;
	char cmd[INPUT_MAX];
	Jobs* temp;
	pid_t* pidList;
	if (index == 1) {
		if (jobs == NULL) {
			printf("fg: no such job\n");
		} else {
			temp = jobs->next;
			wakeChildren(jobs->pidList);
			strcpy(cmd, jobs->cmd);
			printf("Job wake up: %s\n", cmd);
			pidList = jobs->pidList;
			free(jobs);
			//jobs = temp;
			jobs = waitChildren(pidList, temp, cmd);
		}
	} else {
		temp = jobs;
		if (temp == NULL) {
			printf("fg: no such job\n");
			return jobs;
		}
		for(i = 0;temp !=NULL && i != index-2;i++) {
			if (temp->next != NULL) {
				temp=temp->next;
			} else {
				printf("fg: no such job\n");
				return jobs;
			}
		}
		if(temp->next != NULL) {
			Jobs* delPtr = temp->next;
			temp->next = temp->next->next;
			wakeChildren(delPtr->pidList);
			strcpy(cmd, delPtr->cmd);
			printf("Job wake up: %s\n", cmd);
			pidList = delPtr->pidList;
			free(delPtr);
			jobs = waitChildren(pidList, jobs, cmd);
		} else {
			printf("fg: no such job\n");
			return jobs;
		}
	}
	return jobs;
}
Jobs* waitChildren(pid_t* childPid, Jobs* jobs, char* cmdl) {
	int childStatus;
	int i;
	tcsetpgrp(STDIN_FILENO, childPid[0]);
	tcsetpgrp(STDOUT_FILENO, childPid[0]);
	for(i = 0; childPid[i] != 0; i++) {
		DEBUG(printf("waiting for %d\n", childPid[i]););
		waitpid(childPid[i], &childStatus, WUNTRACED);
		if (WIFSTOPPED(childStatus)) {
			if (i == 0) {
				Jobs* newNode = malloc(sizeof(Jobs));
				strcpy(newNode->cmd, cmdl);
				newNode->pidList = childPid;
				newNode->next = NULL;
				if (jobs == NULL) {
					DEBUG(printf("first node added\n"););
					jobs = newNode;
				} else {
					Jobs* temp = jobs;
					while (temp -> next) {
						temp = temp->next;
					}
					temp->next = newNode;
				}
			}
		} else {
			if (i == 0) {
				free(childPid);
			}
		}
		DEBUG(
			if (WIFSIGNALED(childStatus))
			printf("#DEBUG# Child was terminated by signal %d.\n",
			WTERMSIG(childStatus));
		);
	}
	tcsetpgrp(STDIN_FILENO, getpid());
	tcsetpgrp(STDOUT_FILENO, getpid());
	return jobs;
}

void wakeChildren(pid_t* childPid) {
	int i = 0;
	while (childPid[i] != 0) {
		kill(childPid[i],SIGCONT);
		i++;
	}
}
