// NAME: Jiaping Zeng
// EMAIL: jiapingzeng@ucla.edu
// ID: 905363270

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
int exitcode;

void parse_options(int argc, char **argv);
void set_input_mode();
void reset_input_mode();
void read_data(int fd, __pid_t pid);
void if_error(int error, char *message);
void handle_shell_exit(int i, __pid_t pid);
void handle_sigpipe();

int main(int argc, char **argv)
{
    parse_options(argc, argv);
    set_input_mode();
    if (shell_flag)
    {
        exitcode = pipe(fwd);
        if_error(exitcode, "Forward pipe failed");
        exitcode = pipe(bwd);
        if_error(exitcode, "Backward pipe failed");
        signal(SIGPIPE, handle_sigpipe);
        __pid_t pid = fork();
        if_error(pid, "Fork failed");
        if (pid > 0)
        {
            // parent
            int retval, i;
            struct pollfd pfds[2];

            exitcode = close(fwd[0]);
            if_error(exitcode, "Unable to close read end of forward pipe");
            exitcode = close(bwd[1]);
            if_error(exitcode, "Unable to close write end of backward pipe");

            pfds[0].fd = STDIN_FILENO;
            pfds[1].fd = bwd[0];
            pfds[0].events = POLLIN | POLLHUP | POLLERR;
            pfds[1].events = POLLIN | POLLHUP | POLLERR;

            while (1)
            {
                retval = poll(pfds, (nfds_t)2, 0);
                if_error(retval, "Poll failed");
                for (i = 0; i < 2; i++)
                {
                    if (pfds[i].revents & POLLIN)
                        read_data(pfds[i].fd, pid);
                    if (pfds[i].revents & (POLLHUP | POLLERR))
                        handle_shell_exit(i, pid);
                }
            }
        }
        else if (pid == 0)
        {
            // child
            exitcode = close(fwd[1]);
            if_error(exitcode, "Unable to close write end of forward pipe");
            exitcode = dup2(fwd[0], STDIN_FILENO);
            if_error(exitcode, "Unable to dup forward read to stdin");
            exitcode = dup2(bwd[1], STDOUT_FILENO);
            if_error(exitcode, "Unable to dup backward write to stdout");
            exitcode = dup2(bwd[1], STDERR_FILENO);
            if_error(exitcode, "Unable to dup backward write to stderr");
            exitcode = execl("/bin/bash", "bash", NULL);
            if_error(exitcode, "Execl failed");
        }
    }
    else
    {
        char buffer[8], ch;
        int bytes_read, i;
        // read keypresses
        while (1)
        {
            bytes_read = read(STDIN_FILENO, &buffer, 8);
            if_error(bytes_read, "Unable to read from stdin");
            for (i = 0; i < bytes_read; i++)
            {
                ch = buffer[i];
                switch (ch)
                {
                case '\003':
                case '\004':
                    exit(0);
                    break;
                case '\r':
                case '\n':
                    exitcode = write(STDOUT_FILENO, "\r\n", 2);
                    if_error(exitcode, "Unable to write CRLF to stdout");
                    break;
                default:
                    exitcode = write(STDOUT_FILENO, &ch, 1);
                    if_error(exitcode, "Unable to write buffer to stdout");
                    break;
                }
            }
        }
    }
    exit(0);
}

void parse_options(int argc, char **argv)
{
    int c, option_index;
    static struct option long_options[] = {
        {"shell", no_argument, &shell_flag, 's'},
        {"debug", no_argument, &debug_flag, 'd'},
        {0, 0, 0, 0}};
    while (1)
    {
        c = getopt_long(argc, argv, "sd", long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
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

void set_input_mode()
{
    struct termios newconfig;
    // set input mode
    exitcode = tcgetattr(STDIN_FILENO, &oldconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    atexit(reset_input_mode);
    exitcode = tcgetattr(STDIN_FILENO, &newconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);
    if_error(exitcode, "Unable to set terminal parameters");
}

void reset_input_mode()
{
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
    if_error(exitcode, "Unable to set terminal parameters");
}

void read_data(int fd, __pid_t pid)
{
    int i;
    int bytes_read;
    char buffer[256];
    bytes_read = read(fd, buffer, 256);
    if_error(bytes_read, "Unable to read buffer");
    if (fd == STDIN_FILENO)
    {
        // terminal
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\003':
                exitcode = kill(pid, SIGINT);
                if_error(exitcode, "Unable to kill child process");
                break;
            case '\004':
                exitcode = close(fwd[1]);
                if_error(exitcode, "Unable to close write end of forward pipe");
                break;
            case '\r':
            case '\n':
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write CRLF to stdout");
                exitcode = write(fwd[1], "\n", 1);
                if_error(exitcode, "Unable to write LF to forward pipe");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write buffer to stdout");
                exitcode = write(fwd[1], &buffer[i], 1);
                if_error(exitcode, "Unable to write buffer to forward pipe");
                break;
            }
        }
    }
    else
    {
        // shell
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\r':
            case '\n':
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write CRLF to stdout");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write buffer to stdout");
                break;
            }
        }
    }
}

void handle_shell_exit(int i, __pid_t pid)
{
    int status;
    if (i == 0)
    {
        exitcode = kill(pid, SIGINT);
        if_error(exitcode, "Unable to kill child process");
    }
    exitcode = waitpid(pid, &status, 0);
    if_error(exitcode, "Waitpid failed");
    fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\r\n", WTERMSIG(status), WEXITSTATUS(status));
    close(bwd[0]);
    exit(0);
}

void handle_sigpipe()
{
    fprintf(stderr, "Received SIGPIPE\n");
    exit(0);
}

void if_error(int error, char *message)
{
    if (error < 0)
    {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}