#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

struct termios oldconfig;

void set_input_mode();
void reset_input_mode();

int main(int argc, char **argv) {

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

    char ch;
    set_input_mode();
    while (1) {
        read(STDIN_FILENO, &ch, 1);
        if (ch == '\004')
            break;
        else if (ch == '\r' || ch == '\n')
            write(STDOUT_FILENO, "\r\n", 2);
        else
            write(STDOUT_FILENO, &ch, 1);
    }

    return 0;
}

void set_input_mode() {
    struct termios newconfig;
    tcgetattr(STDIN_FILENO, &oldconfig);
    atexit(reset_input_mode);
    tcgetattr(STDIN_FILENO, &newconfig);
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);
}

void reset_input_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
}