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

static int alloc_size;
static char *memory;

void segv_handler(int signal_number)
{
	mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
}

int main(int argc,char *argv[])
{
	int fd;
	int childnum;
	struct sigaction sa;

	printf("Start loop ...\n");
    printf("pid: %d\n", getpid());

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
    printf("Start tracing ...\n");

    while (1){
        mprotect(memory, alloc_size, PROT_READ);
        memory[0] = 1;
        sleep(1);
    }
	
	munmap(memory, alloc_size);
	return 0;
}