#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>

#define RANGE 4096   // task access range: [0,4096)

static int alloc_size;
static char *memory;
static int times;

void segv_handler(int signal_number)
{
	mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
}

int main()
{
	int fd;
	int childnum;
	struct sigaction sa;

	printf("Start RAS scheduler test ...\n");

    // Init segv_handler to handle SIGSEGV for wcounts tracing.
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &segv_handler;
	sigaction(SIGSEGV, &sa, NULL);

    // Allocate memory for process, set the memory can only be read.
	alloc_size = 10 * getpagesize();
	fd = open("/dev/zero", O_RDONLY);
	memory = mmap(NULL, alloc_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	printf("Please choose the number of child tasks (1-100): ");

	scanf("%d",&childnum)

	if (childnum < 1 || childnum > 100){
		printf("invalid child num!\n");
		return -1;
	}

	int i;
	pid_t pid;
	pid_t *child_pid_arr;
	child_pid_arr = malloc(sizeof(pid_t) * childnum);
	for (i = 0;i < childnum; i++){
		// Fork child task.
		pid = fork();
		if (pid == 0){
			// child task

		}else if (pid > 0){
			// parent task
			child_pid_arr[i] = pid;
		}
	}

	// Wait for all child tasks to finish.
	for (i = 0; i < childnum; i++)
		waitpid(child_pid_arr[i],NULL,0);

	syscall(361, getpid());

	/* set protection */
	mprotect(memory, alloc_size, PROT_READ);
	/* try to write, will receive a SIGSEGV */
	memory[0] = 0;
	printf("memory[0] = %d\n",memory[0]);
	
	/* set protection */
	mprotect(memory, alloc_size, PROT_READ);
	/* try to write, will receive a SIGSEGV */
	memory[0] = 1;
	printf("memory[0] = %d\n",memory[0]);

	/* Get wcount */
	syscall(363, getpid(), &wcount);
	printf("Task pid : %d, Wcount = %d, times = %d\n", getpid(), wcount, times);	
	
	/* freee */
	munmap(memory, alloc_size);
	return 0;
}