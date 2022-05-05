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

#define RANGE 4096 // task access range: [0,4096)

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

	scanf("%d", &childnum);

	if (childnum < 1 || childnum > 100)
	{
		printf("invalid child num!\n");
		return -1;
	}

	
	pid_t pid;
	pid_t *child_pid_arr;
	child_pid_arr = malloc(sizeof(pid_t) * childnum);
	int i;
	for (i = 0; i < childnum; i++)
	{
		// Fork child task.
		pid = fork();
		if (pid > 0)
		{
			// parent task
			child_pid_arr[i] = pid;
			printf("Fork child pid: %d\n", pid);
		} 
		else if (pid == 0)
		{
			// child task
			syscall(361, getpid());
			struct sched_param param;
			param.sched_priority = 0;
			syscall(156, getpid(), 6, &param);
			int policy = syscall(157, getpid());
			int j;
			for (j = 0; j < RANGE * 10; j++)
			{
				mprotect(memory, alloc_size, PROT_READ);
				memory[j%RANGE] = j;
			}

			struct timespec t;
			long timeslice;
			syscall(161, getpid(), &t);
			timeslice = t.tv_nsec / 1000000;
			printf("Pid: %d Timeslice : %ld ms.\n", getpid(), timeslice);
			exit(1);
		}
	}

	// Wait for all child tasks to finish.
	for (i = 0; i < childnum; i++)
		waitpid(child_pid_arr[i],NULL,0);

	printf("END!\n");
	/* freee */
	munmap(memory, alloc_size);
	return 0;
}
