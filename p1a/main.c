#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main()
{
    int fwd[2];
    int bwd[2];
    char *str = "ps";
    //char buffer1[256];
    char buffer2[256];
    int status;

    pipe(fwd);
    pipe(bwd);

    __pid_t pid = fork();

    if (pid > 0)
    {
        // parent
        close(fwd[0]);
        write(fwd[1], str, strlen(str)+1);

        waitpid(pid, &status, 0);

        close(bwd[1]);

        read(bwd[0], buffer2, sizeof(buffer2));
        printf("Parent received: \n%s\n", buffer2);

        exit(0);
    }
    else
    {
        // child
        //close(fwd[1]);

        //read(fwd[0], buffer1, sizeof(buffer1));
        //printf("Child: %s\n", buffer1);

        //write(STDIN_FILENO, buffer1, sizeof(buffer1));

        dup2(STDIN_FILENO, fwd[0]);
        dup2(bwd[1], STDOUT_FILENO);
        dup2(bwd[1], STDERR_FILENO);

        execl("/bin/sh", "sh", NULL);
    }
}