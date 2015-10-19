#define INPUT_MAX 255	// Max cmd line input length.
typedef struct jobs {
char cmd[INPUT_MAX];
pid_t *pidList;
struct jobs *next;
} Jobs;
Jobs* cdMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs);

Jobs* exitMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs);

Jobs* fgMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs);

Jobs* jobsMain(int cmdlArgc, char **cmdlArgv, Jobs* jobs);

Jobs* waitChildren(pid_t* children, Jobs* jobs);

Jobs* wakeJob(Jobs* jobs, int index);

void wakeChildren(pid_t* children);
