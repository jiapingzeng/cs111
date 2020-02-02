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
#include <signal.h>
#include "zlib.h"

#define BUFFER_SIZE 256

__pid_t pid;
static int compress_flag;
int port, exitcode, sfd, newsfd, fwd[2], bwd[2];
z_stream def_stream, inf_stream;

void read_data(int fd, __pid_t pid);
int def(char *src, char *dest, int src_size, int dest_size);
int inf(char *src, char *dest, int src_size, int dest_size);
void parse_options(int argc, char **argv, int *port);
void if_error(int error, char *message);
void handle_sigpipe();
void handle_sigint();
void exit_cleanup();
void init();

int main(int argc, char **argv)
{
    parse_options(argc, argv, &port);
    init();

    printf("PORT: %d\n", port);

    pid = fork();
    if_error(pid, "Fork failed");
    if (pid > 0)
    {
        // parent (terminal)
        unsigned int i;
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
        // child (shell)
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
    unsigned int i, bytes_read;
    char buffer[BUFFER_SIZE], cbuffer[BUFFER_SIZE];
    bytes_read = read(fd, buffer, BUFFER_SIZE);
    if_error(bytes_read, "Unable to read buffer");
    if (fd == newsfd)
    {
        if (compress_flag)
        {
            memcpy(cbuffer, buffer, bytes_read);
            bytes_read = inf(cbuffer, buffer, bytes_read, BUFFER_SIZE);
        }
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
                exitcode = write(STDOUT_FILENO, "\n", 1);
                if_error(exitcode, "Unable to write LF to stdout");
                exitcode = write(fwd[1], "\n", 1);
                if_error(exitcode, "Unable to write LF toforward pipe");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write to stdout");
                exitcode = write(fwd[1], &buffer[i], 1);
                if_error(exitcode, "Unable to write forward pipe");
                break;
            }
        }
    }
    else
    {
        if (compress_flag)
        {
            memcpy(cbuffer, buffer, bytes_read);
            bytes_read = def(cbuffer, buffer, bytes_read, BUFFER_SIZE);
        }
        exitcode = write(newsfd, &buffer, bytes_read);
        if_error(bytes_read, "Unable to write to new socket file descriptor");
    }
}

int def(char *src, char *dest, int src_size, int dest_size)
{
    def_stream.avail_in = (uInt)src_size;
    def_stream.next_in = (Bytef *)src;
    def_stream.avail_out = (uInt)dest_size;
    def_stream.next_out = (Bytef *)dest;
    do
    {
        deflate(&def_stream, Z_SYNC_FLUSH);
    } while (def_stream.avail_in > 0);
    return dest_size - def_stream.avail_out;
}

int inf(char *src, char *dest, int src_size, int dest_size)
{
    inf_stream.avail_in = (uInt)src_size;
    inf_stream.next_in = (Bytef *)src;
    inf_stream.avail_out = (uInt)dest_size;
    inf_stream.next_out = (Bytef *)dest;
    do
    {
        inflate(&inf_stream, Z_SYNC_FLUSH);
    } while (inf_stream.avail_in > 0);
    return dest_size - inf_stream.avail_out;
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

void init()
{
    // set up compression
    if (compress_flag)
    {
        def_stream.zalloc = Z_NULL;
        def_stream.zfree = Z_NULL;
        def_stream.opaque = Z_NULL;
        exitcode = deflateInit(&def_stream, Z_DEFAULT_COMPRESSION);
        if_error(exitcode, "Deflate init failed");
        inf_stream.zalloc = Z_NULL;
        inf_stream.zfree = Z_NULL;
        inf_stream.opaque = Z_NULL;
        exitcode = inflateInit(&inf_stream);
        if_error(exitcode, "Inflate init failed");
    }

    // set up sockets
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

    // set up pipes
    exitcode = pipe(fwd);
    if_error(exitcode, "Forward pipe failed");
    exitcode = pipe(bwd);
    if_error(exitcode, "Backward pipe failed");

    // set up signals
    signal(SIGPIPE, handle_sigpipe);
    signal(SIGINT, handle_sigint);

    // set up cleanup on exit
    atexit(exit_cleanup);
}

void if_error(int error, char *message)
{
    if (error < 0)
    {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}

void handle_sigpipe()
{
    fprintf(stderr, "Caught SIGPIPE\n");
    exit(0);
}

void handle_sigint()
{
    exitcode = kill(pid, SIGINT);
    if_error(exitcode, "Unable to kill child process");
}

void exit_cleanup()
{
    int status;
    exitcode = waitpid(pid, &status, 0);
    if_error(exitcode, "Waitpid failed");
    fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\r\n", WTERMSIG(status), WEXITSTATUS(status));
    close(bwd[0]);
    close(sfd);
    close(newsfd);

    // deflate zstreams
    if (compress_flag)
    {
        deflateEnd(&def_stream);
        deflateEnd(&inf_stream);
    }
}