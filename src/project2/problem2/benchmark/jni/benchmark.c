#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

#define WRITE_RANGE 1024
#define MAX_WRITE_TIMES 8192
#define SHIFT_RANGE 10

static int alloc_size;
static char *memory;
static int times;

void segv_handler(int signal_number)
{
    mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
}

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));

    int fd;
    int childnum;
    struct sigaction sa;

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

    pid_t *child_pid_arr;
    int *write_times_arr;
    int write_sum;
    child_pid_arr = malloc(sizeof(pid_t) * childnum);
    write_times_arr = malloc(sizeof(int) * childnum);
    write_sum = 0;

    printf("> Start  NORMAL | FIFO | RR | RAS  Benchmark ...\n");

    int i;
    for (i = 0; i < childnum; i++)
    {
        write_times_arr[i] = MAX_WRITE_TIMES >> (rand() % SHIFT_RANGE);
        write_sum += write_times_arr[i];
    }

    printf("> Random write start, write times sum: %d ...\n", write_sum);

    int sched;
    pid_t pid;
    int write_times;

    int sched_arr[4] = {0, 1, 2, 6};
    char *sched_name_arr[4] = {"NORMAL", "FIFO", "RR", "RAS"};

    struct timeval start;
    struct timeval end;

    int time_us;

    double time_consume_arr[4];
    double writes_per_sec_arr[4];

    for (sched = 0; sched < 4; sched++)
    {
        printf("--------------------------------------------------\n");
        printf("> Start %s Benchmark ...\n", sched_name_arr[sched]);
        gettimeofday(&start, NULL);

        for (i = 0; i < childnum; i++)
        {
            // Fork child task.
            write_times = write_times_arr[i];
            pid = fork();
            if (pid > 0)
            {
                // parent task
                child_pid_arr[i] = pid;
                printf("[%s] Fork child pid: %d\n",sched_name_arr[sched], pid);
            }
            else if (pid == 0)
            {
                // child task
                int oldpolicy;
                oldpolicy = syscall(157, getpid());
                syscall(361, getpid());
                struct sched_param param;
                param.sched_priority = sched_arr[sched] == 6 ? 0 : 40;
                syscall(156, getpid(), sched_arr[sched], &param);
                printf("[%s] Set task pid %d policy %d -> %d\n",sched_name_arr[sched], getpid(), oldpolicy, sched_arr[sched]);
                sleep(5);

                int j;
                for (j = 0; j < write_times; j++)
                {
                    int pos;
                    pos = rand() % WRITE_RANGE;
                    mprotect(memory, alloc_size, PROT_READ);
                    memory[pos] = pos;
                }

                // struct timespec t;
                // long timeslice;
                // syscall(161, getpid(), &t);
                // timeslice = t.tv_nsec / 1000000;
                printf("[%s] Pid %d done, write times: %d\n",sched_name_arr[sched], getpid(), write_times);
                exit(0);
            }
            else
            {
                printf("Fork error\n");
                exit(1);
            }
        }

        // Wait for all child tasks to finish.
        for (i = 0; i < childnum; i++)
            waitpid(child_pid_arr[i], NULL, 0);

        gettimeofday(&end, NULL);
        time_us = 1000000 * (end.tv_sec - start.tv_sec - 5) + (end.tv_usec - start.tv_usec);
        double writes_per_sec;
        writes_per_sec = write_sum * 1.0 / (time_us * 1.0 / 1000000);

        time_consume_arr[sched] = time_us*1.0 / 1000000;
        writes_per_sec_arr[sched] = writes_per_sec;

        printf("> %s benchmark end, time consume: %.3f s, write %.2f times per sec\n", sched_name_arr[sched], time_consume_arr[sched], writes_per_sec);
        printf("--------------------------------------------------\n");
    }

    printf("> All benchmarks completed.\n");
    printf("> Statistical information:\n");
    printf("Total write times: %d, child process count: %d\n", write_sum, childnum);
    printf("--------------------------------------------------\n");
    printf("SCHED\t time consume\t write times per sec\t\n");
    for (sched = 0; sched < 4; sched++)
    {
        printf("%s \t %.3fs\t %.2f\t\n", sched_name_arr[sched], time_consume_arr[sched], writes_per_sec_arr[sched]);
    }
    printf("--------------------------------------------------\n");
    printf("> Benchmark end.\n");

    // free
    munmap(memory, alloc_size);
    free(child_pid_arr);
    free(write_times_arr);
    return 0;
}
