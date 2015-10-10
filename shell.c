#include <stdio.h>
#include <string.h>	// strerror, strchr, strlen, strtok_r
#include <stdlib.h>	// malloc, free, exit
#include <errno.h>	// errno
#include <unistd.h>	// getcwd
#include "shell.h"
#include "debug.h"

/*	Gets CWD and put into cwd if current size cwdBufLen is enough.
 *	Else allocates larger array storing CWD then returns char * to it.
 *	Users should not free the returned cwd. */
char *getCwdDynamicBuf() {
	static int cwdBufLen = 128;	// Initial buffer length will be double of this.
	static char *cwd = NULL;
	char *buf;
	while (cwd == NULL || getcwd(cwd, cwdBufLen) == NULL) {
		cwdBufLen *= 2;
		DEBUG(printf("#DEBUG# cwdBufLen too short. Trying %d.\n", cwdBufLen););
		if ((buf = malloc(sizeof(char) * cwdBufLen)) == NULL) {
			fprintf(stderr, "Error %d: %s for cwd.\n", errno, strerror(errno));
			exit(1);
		}
		free(cwd);
		cwd = buf;
	}
	return cwd;
}

/*	Displays cmd line prompt.
 *	Gets cmd line from stdin and store in cmdl.
 *	Max cmd line length is specified by INPUT_MAX in shell.h. */
void cmdLinePrompt(char *const cmdl) {
	char buf[8], *newlinePtr;
	printf("[3150 shell:%s]$ ", getCwdDynamicBuf());
	if (fgets(cmdl, INPUT_MAX + 1, stdin) == NULL) {	// EOF received.
		DEBUG(printf("\n#DEBUG# EOF received."););
		printf("\n");
		exit(0);
	}
	if ((newlinePtr = strchr(cmdl, '\n')) != NULL)
		*newlinePtr = '\0';
			// When input len == INPUT_MAX, no '\n' to remove!
	while (newlinePtr == NULL) {
		fgets(buf, 8, stdin);
		newlinePtr = strchr(buf, '\n');
	}		// Flush '\n' remained in stdin when input len == INPUT_MAX!
			// Code reuse-able if limit on INPUT_MAX waived.
	DEBUG(printf("#DEBUG# cmdl=%s=END\n", cmdl););
}

/*	Tokenize the cmd line cmdl, delimiting by ' '.
 *	Returns an array of strings storing the tokens.
 *	The array has an extra NULL entry at the end
 *	and should be freed using freeCmdlArgv() when not needed.
 *	The number of tokens is stored in *cmdlArgc. */
char ***tokenizeCmdl(char *const cmdl, int **const cmdlArgc, int *const processCount) {
	char ***cmdlArgv, *savePtr;	// savePtr is for strtok_r.
	/** Count num of tokens separated by ' ' **/
	int i = 0, j = 0, cmdlLen = strlen(cmdl);
	int processNow = 0;
	*processCount = 1;
	for (i = 0; i < cmdlLen; i++) {
		if (cmdl[i] == '|') {
			(*processCount)++;
		}
	}
	*cmdlArgc = calloc(*processCount, sizeof(int));
	i = 0;
	while (i < cmdlLen) {
		while (cmdl[i] != '|' && i < cmdlLen) {
			while (cmdl[i] == ' ' && i < cmdlLen)
				i++;
			if (cmdl[i] == '|')
				break;
			if (i < cmdlLen)
				((*cmdlArgc)[processNow])++;
			while (cmdl[i] != ' ' && cmdl[i] != '|' && i < cmdlLen)
				i++;
		}
		processNow++;
		i++;
	}
	/** Allocate arg table and register tokens into it **/
	if ((cmdlArgv = malloc(sizeof(char **) * (*processCount))) == NULL) {
		fprintf(stderr, "Error %d: %s for cmdlArgv.\n", errno, strerror(errno));
		exit(1);
	}
	processNow = 0;
	while (processNow < *processCount) {
		j = 0;
		cmdlArgv[processNow] = malloc(sizeof(char *) * ((*cmdlArgc)[processNow] + 1));
		if (cmdlArgv[processNow] == NULL) {
			// allocate error
			exit(1);
		}
		while (j < (*cmdlArgc)[processNow]) {
			if (processNow == 0 && j == 0) {
				cmdlArgv[processNow][j] = strtok(cmdl, " |");
			} else {
				cmdlArgv[processNow][j] = strtok(NULL, " |");
			}
			j++;
		}
		cmdlArgv[processNow][j] = NULL;
		processNow++;
	}
	DEBUG(
		printf("#DEBUG# processCount=%d\n", *processCount);
		for (i = 0; i < *processCount; i++) {
			for (j = 0; cmdlArgv[i][j] != NULL; j++)
				printf("#DEBUG# cmdlArgv[%d]=%s=END\n", j, cmdlArgv[i][j]);
			printf("#DEBUG# cmdlArgv[%d]=%s=END\n", j, cmdlArgv[i][j]);
		}
	);
	return cmdlArgv;
}

/*	Frees and set to NULL the cmdlArgv returned by tokenizeCmdl(). */
void freeCmdlArgv(char ***cmdlArgv, int processCount) {
	int i = 0;
	for(;i<processCount;i++) {
		free(cmdlArgv[i]);
	}
	free(cmdlArgv);
	cmdlArgv = NULL;
}
