#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

static int compress_flag;
int exitcode, sfd, newsfd, fwd[2], bwd[2];

void read_data(int fd, __pid_t pid);
void parse_options(int argc, char **argv, int *port);
void if_error(int error, char *message);

int main(int argc, char **argv)
{
    int port;

    parse_options(argc, argv, &port);

    printf("PORT: %d\n", port);

    struct sockaddr_in addr;
    unsigned int len;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if_error(sfd, "Unable to open socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    len = sizeof(addr);

    exitcode = bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
    if_error(exitcode, "Bind failed");

    exitcode = listen(sfd, 5);
    if_error(exitcode, "Listen failed");

    newsfd = accept(sfd, (struct sockaddr *)&addr, (socklen_t *)&len);
    if_error(exitcode, "Accept failed");
/*
    int bytes_read;
    char buffer[1024];
    bytes_read = read(newsfd, &buffer, 1024);
    if_error(bytes_read, "Unable to read from new socket");

    write(STDOUT_FILENO, &buffer, bytes_read);

    char *test = "\nServer test\n";
    send(newsfd, test, strlen(test), 0);

    printf("Server sent message successfully\n");
*/
    exitcode = pipe(fwd);
    if_error(exitcode, "Forward pipe failed");
    exitcode = pipe(bwd);
    if_error(exitcode, "Backward pipe failed");

    __pid_t pid = fork();
    if_error(pid, "Fork failed");
    if (pid > 0)
    {
        int i;
        struct pollfd pfds[2];
        exitcode = close(fwd[0]);
        if_error(exitcode, "Unable to close read end of forward pipe");
        exitcode = close(bwd[1]);
        if_error(exitcode, "Unable to close write end of backward pipe");

        pfds[0].fd = newsfd;
        pfds[1].fd = bwd[0];
        pfds[0].events = POLLIN | POLLHUP | POLLERR;
        pfds[1].events = POLLIN | POLLHUP | POLLERR;

        while (1)
        {
            exitcode = poll(pfds, 2, 0);
            if_error(exitcode, "Poll failed");
            for (i = 0; i < 2; i++)
            {
                if (pfds[i].revents & POLLIN)
                    read_data(pfds[i].fd, pid);
                if (pfds[i].revents & (POLLHUP | POLLERR))
                    exit(0);
            }
        }
    }
    else
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
    exit(0);
}

void read_data(int fd, __pid_t pid)
{
    int i, bytes_read;
    char buffer[256];
    bytes_read = read(fd, buffer, 256);
    if_error(bytes_read, "Unable to read buffer");
    if (fd == newsfd)
    {
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
                exitcode = write(fwd[1], "\n", 1);
                if_error(exitcode, "Unable to write forward pipe");
                break;
            default:
                exitcode = write(fwd[1], &buffer[i], 1);
                if_error(exitcode, "Unable to write forward pipe");
                break;
            }
        }
    }
    else
    {
        exitcode = write(newsfd, &buffer, bytes_read);
        if_error(bytes_read, "Unable to write to new socket file descriptor");
    }
}

void parse_options(int argc, char **argv, int *port)
{
    int c;
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"compress", no_argument, &compress_flag, 'c'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "pc", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 0:
            // set flag
            break;
        case 'p':
            *port = atoi(optarg);
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

void if_error(int error, char *message)
{
    if (error < 0)
    {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}