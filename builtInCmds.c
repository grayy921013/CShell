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
	if (jobs != NULL) {
		printf("There is at least one suspended job\n");
		return jobs;
	} else {
		exit(0);
	}
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
	Jobs* temp;
	temp = jobs;
	for (;i < index-1;i++) {
		temp = temp->next;
	}
	if (temp == NULL) {
		printf("fg: no such job\n");
		return jobs;
	} else {
		printf("Job wake up: %s\n", temp->cmd);
		wakeChildren(temp->pidList);
		jobs = waitChildren(temp->pidList, jobs);
	}
	return jobs;
}
Jobs* waitChildren(pid_t* childPid, Jobs* jobs) {
	int childStatus;
	int childCount = 0;
	int i;
	tcsetpgrp(STDIN_FILENO, childPid[0]);
	tcsetpgrp(STDOUT_FILENO, childPid[0]);
	while (childPid[childCount] != 0) {
		DEBUG(printf("childpid: %d\n", childPid[childCount]););
		childCount++;
	}
	for(i = 0; childPid[i] != 0; i++) {
		DEBUG(printf("waiting for %d\n", childPid[i]););
		waitpid(childPid[i], &childStatus, WUNTRACED);
		DEBUG(
			if (WIFSIGNALED(childStatus))
			printf("#DEBUG# Child was terminated by signal %d.\n",
			WTERMSIG(childStatus));
		);
	}
	if (!WIFSTOPPED(childStatus)) {
		//delete the node
		Jobs* temp;
		if (jobs->pidList[0] == childPid[0]) {
			//first node
			temp = jobs->next;
			free(jobs->pidList);
			free(jobs);
			jobs = temp;
		} else {
			//not first node
			Jobs* delPtr;
			temp = jobs;
			while(temp->next->pidList[0] != childPid[0]) {
				temp = temp->next;
			}
			delPtr = temp->next;
			temp->next = temp->next->next;
			free(delPtr->pidList);
			free(delPtr);
		}
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
