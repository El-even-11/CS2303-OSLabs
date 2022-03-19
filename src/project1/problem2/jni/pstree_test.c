#include <stdio.h>

#define __NR_pstreecall 356
#define __MAX_BUFSIZE 1024
#define __MAX_PSTREE_DEPTH 512

struct prinfo
{
    pid_t parent_pid;       // pid of parent
    pid_t pid;              // pid of this process
    pid_t first_child_pid;  // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state;             // current state of process
    long uid;               // user id of process owner
    char comm[64];          // name of program executed
};

void print_tree(struct prinfo *buf, int *nr)
{
    int i;
    int depth = -1;
    // parent_pids : a stack to store parents' pids;
    int parent_pids[__MAX_PSTREE_DEPTH];
    for (i = 1; i < *nr; i++)
    {
        int parent_pid = buf[i].parent_pid;
        if (parent_pid == 0)
        {
            depth = 0;
        }
        else if (parent_pid != parent_pids[depth])
        {
            // parent process changed
            int j;
            for (j = 1; j < depth; j++)
            {
                if (parent_pid == parent_pids[j])
                {
                    break;
                }
            }
            if (j >= depth)
            {
                // new parent process
                depth++;
                parent_pids[depth] = parent_pid;
            }
            else
            {
                // former parent process
                depth = j;
            }
        }

        int j;
        for (j = 0; j < depth; j++)
        {
            printf("\t");
        }

        printf("%s,%d,%ld,%d,%d,%d,%d\n", buf[i].comm, buf[i].pid, buf[i].state, buf[i].parent_pid, buf[i].first_child_pid, buf[i].next_sibling_pid, buf[i].uid);
    }
}

int main()
{
    struct prinfo *buf = malloc(sizeof(struct prinfo) * __MAX_BUFSIZE);
    int *nr = malloc(sizeof(int));
    *nr = 0;

    int ok = syscall(__NR_pstreecall, buf, nr);
    if (ok != 0)
    {
        printf("system call pstree failed, code = %d.\n", ok);
        return 1;
    }
    print_tree(buf, nr);

    free(buf);
    free(nr);

    return 0;
}