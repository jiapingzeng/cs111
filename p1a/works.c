#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>

static int shell_flag;
static int debug_flag;
struct termios oldconfig;
int fwd[2], bwd[2];

void parse_options(int argc, char **argv);
void run_terminal();
void reset_input_mode();
void read_data(int fd, __pid_t pid);

int main(int argc, char **argv)
{
    parse_options(argc, argv);
    if (shell_flag) {

        if (pipe(fwd) == -1 || pipe(bwd) == -1) {
            fprintf(stderr, "Pipe failed\n");
            exit(1);
        }

        __pid_t pid = fork();
        if (pid > 0) {
            // parent
            int retval, i;
            struct pollfd pfds[2];

            close(fwd[0]);
            close(bwd[1]);

            pfds[0].fd = STDIN_FILENO;
            pfds[1].fd = bwd[0];
            pfds[0].events = POLLIN | POLLHUP | POLLERR;
            pfds[1].events = POLLIN | POLLHUP | POLLERR;

            while (1) {
                retval = poll(pfds, (nfds_t)2, 0);
                if (retval < 0) {
                    fprintf(stderr, "Unable to create poll\n");
                    exit(1);
                }
                for (i = 0; i < 2; i++) {
                    // can read
                    if (pfds[i].revents & POLLIN)
                        read_data(pfds[i].fd, pid);
                    else if (pfds[i].revents & (POLLHUP | POLLERR)) {
                        if (i == 0) {
                            // keyboard
                            printf("error from keyboard");
                        } else {
                            // shell
                            printf("error from shell");
                        }
                    }
                }
            }
        } else if (pid == 0) {
            // child
            close(fwd[1]);
            dup2(fwd[0], STDIN_FILENO);
            dup2(bwd[1], STDOUT_FILENO);
            dup2(bwd[1], STDERR_FILENO);
            execl("/bin/bash", "bash", NULL);
            fprintf(stderr, "Unable to exec\n");
        } else {
            // error
            fprintf(stderr, "Unable to fork\n");
            exit(1);
        }
    } else {
        run_terminal();
    }
    exit(0);
}

void parse_options(int argc, char **argv) {
    int c, option_index;
    static struct option long_options[] = {
        {"shell", no_argument, &shell_flag, 's'},
        {"debug", no_argument, &debug_flag, 'd'},
        {0, 0, 0, 0}
    };
    while (1) {
        c = getopt_long(argc, argv, "sd", long_options, &option_index);
        if (c == -1) break;
        switch(c) {
            case 0:
                break;
            case '?':
                fprintf(stderr, "Unable to retrieve option\n");
                exit(1);
            default:
                fprintf(stderr, "Unknown option: %d\n", c);
                exit(1);
        }
    }
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

void read_data(int fd, __pid_t pid) {
    int i;
    ssize_t bytes_read;
    char buffer[256];
    bytes_read = read(fd, buffer, 256);
    if (fd == STDIN_FILENO) {
        for (i = 0; i < bytes_read; i++) {
            switch (buffer[i]) {
                case '\003':
                    kill(pid, SIGINT);
                    break;
                case '\004':
                    kill(fwd[1], SIGINT);
                    break;
                case '\r':
                case '\n':
                    write(STDOUT_FILENO, "\r\n", 2);
                    write(fwd[1], "\n", 1);
                    break;
                default:
                    write(STDOUT_FILENO, &buffer[i], 1);
                    write(fwd[1], &buffer[i], 1);
                    break;
            }
        }
    } else {
        for (i = 0; i < bytes_read; i++) {
            switch (buffer[i]) {
                case '\r':
                case '\n':
                    write(STDOUT_FILENO, "\r\n", 2);
                    break;
                default:
                    write(STDOUT_FILENO, &buffer[i], 1);
                    break;
            }
        }
    }
}