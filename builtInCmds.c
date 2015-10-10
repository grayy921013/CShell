#include <stdio.h>
#include <stdlib.h>	// exit
#include <unistd.h>	// chdir
#include "debug.h"

/**	Function names, arguments and return values must follow the convension. **/

/*	Changes current working directory. Accepts only exactly 1 arg. */
int cdMain(int cmdlArgc, char **cmdlArgv) {
	DEBUG(printf("#DEBUG# Calling cd.\n"););
	if (cmdlArgc != 2) {
		printf("cd:  wrong number of arguments\n");
		return 1;
	}
	if (chdir(cmdlArgv[1]) == -1) {
		printf("%s:  cannot change directory\n", cmdlArgv[1]);
		return 2;
	}
	return 0;
}

/*	Exit from the programme. Accepts only 0 args. */
int exitMain(int cmdlArgc, char **cmdlArgv) {
	DEBUG(printf("#DEBUG# Calling exit.\n"););
	if (cmdlArgc != 1) {
		printf("cd:  wrong number of arguments\n");
		return 1;
	}
	exit(0);
}

int fgMain(int cmdlArgc, char **cmdlArgv) {
	DEBUG(printf("#DEBUG# Calling fg.\n"););
	printf("fg to be implemented in phase 2.\n");
	return 0;
}

int jobMain(int cmdlArgc, char **cmdlArgv) {
	DEBUG(printf("#DEBUG# Calling job.\n"););
	printf("job to be implemented in phase 2.\n");
	return 0;
}

