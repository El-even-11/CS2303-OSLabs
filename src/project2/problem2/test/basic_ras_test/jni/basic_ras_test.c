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

static char *policys[] = {
    "SCHED_NORMAL",
    "SCHED_FIFO",
    "SCHED_RR",
    "ERROR",
    "ERROR",
    "ERROR",
    "SCHED_RAS",
};

void segv_handler(int signal_number)
{
	mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
}

int main(int argc,char *argv[])
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

    // Start tracing
	syscall(361, getpid());
    printf("Start trace for task %d\n",getpid());

    int policy;
    policy = syscall(157, getpid());

    if (policy > 6) {
        printf("ERROR POLICY!\n");
        return -1;
    }

    printf("Current scheduling algorithm is %s\n",policys[policy]);

    printf("Please input the choice of scheduling algorithm (0-NORMAL, 1-FIFO, 2-RR, 6-RAS): %d\n",atoi(argv[1]));
    struct sched_param param;
    param.sched_priority = 0;

    printf("Changing ...\n");
    syscall(156, getpid(), atoi(argv[1]), &param);

    policy = syscall(157, getpid());
    printf("Current scheduling algorithm is %s\n", policys[policy]);

    printf("Writing ...\n");

    int i;
    for (i=0;i<RANGE;i++){
        mprotect(memory, alloc_size, PROT_READ);
        memory[i] = i;
    }

    struct timespec t;
    long timeslice;
    syscall(161, getpid(), &t);
    timeslice = t.tv_nsec / 1000000;
    printf("Timeslice : %ld ms.\n", timeslice);

    int wcounts;
	// Get wcounts.
	syscall(363, getpid(), &wcounts);
	printf("Task pid : %d, Wcount = %d\n", getpid(), wcounts);	
	
	/* freee */
	munmap(memory, alloc_size);
	return 0;
}