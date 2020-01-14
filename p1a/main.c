#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main() {
    int fd1[2];
    int fd2[2];
    char fixed_str[] = "test";
    char input_str[100];
    __pid_t pid;

    pipe(fd1);
    pipe(fd2);

    scanf("%s", input_str);
    pid = fork();

    if (pid > 0) {
        // parent
        char concat_str[100];

        close(fd1[0]);
        write(fd1[1], input_str, strlen(input_str)+1);
        close(fd1[1]);
        
        wait(NULL);
        
        close(fd2[1]);
        read(fd2[0], concat_str, 100);
        printf("Concatenated string %s\n", concat_str);
        close(fd2[0]);
    } else if (pid == 0) {
        // child
        close(fd1[1]);

        char concat_str[100];
        read(fd1[0], concat_str, 100);

        int k = strlen(concat_str);
        size_t i;
        for (i=0; i<strlen(fixed_str); i++) {
            concat_str[k++] = fixed_str[i];
        }

        concat_str[k] = '\0';

        close(fd1[0]);
        close(fd2[0]);

        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }
}