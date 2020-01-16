#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

struct termios oldconfig;

void run_terminal();
void reset_input_mode();

int main(/*int argc, char **argv*/) {

    __pid_t pid = fork();
    int pipe1[2]; // read from terminal, write to shell
    int pipe2[2]; // read from shell, write to terminal

    // create pipes and exit on failure
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        fprintf(stderr, "Pipe failed\n");
        exit(1);
    }

    if (pid > 0) {
        // parent (terminal)
        char *input = "pwd\0";
        char buffer[256];
        close(pipe1[0]);
        write(pipe1[1], input, strlen(buffer));
        close(pipe1[1]);

        wait(NULL);
        close(pipe2[1]);
        read(pipe2[0], buffer, 256);
        printf("%s\n", buffer);
        close(pipe2[0]);
        //run_terminal();
    } else if (pid == 0) {
        // child (exec bash)
        close(pipe1[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        close(pipe2[0]);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);
        execl("/bin/bash", "bash", NULL);
    } else {
        // error
        fprintf(stderr, "Unable to fork");
    }

    /**
    if (argc == 2) {
        if (strcmp(argv[1], "--shell") == 0) {
            __pid_t pid = fork();
            int inpipe[2];
            int outpipe[2];

            if (pid == 0) {
                printf("child");
                pipe(inpipe);
                printf("inpipe: %d, %d", inpipe[0], inpipe[1]);
                pipe(outpipe);
                printf("outpipe: %d, %d", outpipe[0], outpipe[1]);
                return 0;
                execl("/bin/bash", "bash", NULL);
                fprintf(stderr, "Failed to execute shell");
            } else {
                printf("parent");
                return 0;
            }
        } else {
            fprintf(stderr, "Invalid argument");
            exit(1);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Invalid number of arguments");
        exit(1);
    }

    run_terminal();
    **/
    return 0;
}

void run_terminal() {
    char ch;
    struct termios newconfig;

    // set input mode
    tcgetattr(STDIN_FILENO, &oldconfig);
    atexit(reset_input_mode);
    tcgetattr(STDIN_FILENO, &newconfig);
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);

    printf("terminal running");

    // read keypresses
    while (1) {
        read(STDIN_FILENO, &ch, 1);
        if (ch == '\004')
            break;
        else if (ch == '\r' || ch == '\n')
            write(STDOUT_FILENO, "\r\n", 2);
        else
            write(STDOUT_FILENO, &ch, 1);
    }
}

void reset_input_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
}