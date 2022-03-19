#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("520021910171Parent: %d\n", getpid());

    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "fork failed.\n");
        return 1;
    }
    else if (pid == 0)
    {
        printf("520021910171Child: %d\n", getpid());
        if (execl("./pstree_test", "pstree_test", NULL))
        {
            fprintf(stderr, "execute pstree failed\n");
            return 1;
        }
    }
    else
    {
        wait(NULL);
    }

    return 0;
}