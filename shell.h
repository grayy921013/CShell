#define INPUT_MAX 255	// Max cmd line input length.

void cmdLinePrompt(char *const cmdl);

char ***tokenizeCmdl(char *const cmdl, int ** cmdlArgc, int *const processCount);

void freeCmdlArgv(char ***cmdlArgv, int processCount);
