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
    char *str = "hello\n";
    char buffer[256];
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
        read(bwd[0], buffer, sizeof(buffer));
        printf("Received: \n%s", buffer);
        exit(0);
    }
    else
    {
        // child
        close(fwd[1]);
        
        // read(fwd[0], buffer, sizeof(buffer));
        // printf("Received: %s", buffer);

        dup2(STDIN_FILENO, fwd[0]);
        dup2(bwd[1], STDOUT_FILENO);

        execlp("/bin/bash", "bash", NULL);
    }
}