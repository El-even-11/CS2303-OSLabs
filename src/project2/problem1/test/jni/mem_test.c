#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int alloc_size;
static char *memory;
static int times;

void segv_handler(int signal_number)
{
	printf("find memory accessed!\n");
	mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
	times++;
	printf("set memory read write!\n");
}

int main()
{
	int fd;
	struct sigaction sa;
	int wcount;

	printf("Start memory trace testing program...\n");

	syscall(361, getpid());
	printf("Invoke sys_start_trace\n");

	/* Init segv_handler to handle SIGSEGV */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &segv_handler;
	sigaction(SIGSEGV, &sa, NULL);

	times = 0;

	/* allocate memory for process, set the memory can only be read */
	alloc_size = 10 * getpagesize();
	fd = open("/dev/zero", O_RDONLY);
	memory = mmap(NULL, alloc_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	int i;
	for (i = 0; i < 4; i++)
	{
		/* set protection */
		mprotect(memory, alloc_size, PROT_READ);
		/* try to write, will receive a SIGSEGV */
		memory[i] = i;
		printf("memory[%d] = %d\n", i,memory[i]);
	}

	/* Get wcount */
	syscall(363, getpid(), &wcount);
	printf("Task pid : %d, wcounts = %d, actual write times = %d\n", getpid(), wcount, times);
	times = 0;

	// stop tracing
	syscall(362, getpid());
	printf("Invoke sys_stop_trace\n");
	for (i = 0; i < 4; i++)
	{
		/* set protection */
		mprotect(memory, alloc_size, PROT_READ);
		/* try to write, will receive a SIGSEGV */
		memory[i] = i;
		printf("memory[%d] = %d\n", i,memory[i]);
	}

	/* Get wcount */
	syscall(363, getpid(), &wcount);
	printf("Task pid : %d, Wcount = %d, actual write times = %d\n", getpid(), wcount, times);
	
	/* freee */
	munmap(memory, alloc_size);
	return 0;
}