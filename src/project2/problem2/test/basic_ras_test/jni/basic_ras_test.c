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

static char *policys[] = {
    "SCHED_NORMAL",
    "SCHED_FIFO",
    "SCHED_RR",
    "ERROR",
    "ERROR",
    "ERROR",
    "SCHED_RAS",
};

int main(int argc,char *argv[])
{
	int fd;
	int childnum;
	struct sigaction sa;

	printf("Start RAS scheduler test ...\n");
    printf("Please input the pid of the test task: ");
    pid_t pid;

    scanf("%d",&pid);
    int policy;
    policy = syscall(157, pid);

    if (policy > 6) {
        printf("ERROR POLICY!\n");
        return -1;
    }

    printf("Current scheduling algorithm is %s\n",policys[policy]);

    printf("Please input the choice of scheduling algorithm (0-NORMAL, 1-FIFO, 2-RR, 6-RAS): ");
    
    scanf("%d",&policy);

    int prio;
    printf("Please input the priority: ");
    scanf("%d",&prio);

    struct sched_param param;
    param.sched_priority = prio;
    

    printf("Changing ...\n");
    syscall(156, pid, policy, &param);

    policy = syscall(157, pid);
    printf("Current scheduling algorithm is %s\n", policys[policy]);
    printf("Done.\n");

	return 0;
}