#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <termios.h>
#include <poll.h>

int exitcode;
static int compress_flag;
struct termios oldconfig;
int sfd;

void read_data(int fd);
void set_input_mode();
void reset_input_mode();
void parse_options(int argc, char **argv, int *port, char **log);
void if_error(int error, char *message);

int main(int argc, char **argv)
{
    int port;
    char *log;

    parse_options(argc, argv, &port, &log);
    set_input_mode();

    int i;
    unsigned int len;
    struct sockaddr_in addr;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if_error(sfd, "Unable to open socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    len = sizeof(addr);

    exitcode = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if_error(exitcode, "Address not supported");

    exitcode = connect(sfd, (struct sockaddr *)&addr, (socklen_t)len);
    if_error(exitcode, "Connection failed");

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[1].fd = sfd;
    pfds[0].events = POLLIN | POLLHUP | POLLERR;
    pfds[1].events = POLLIN | POLLHUP | POLLERR;

    while (1)
    {
        exitcode = poll(pfds, 2, 0);
        if_error(exitcode, "Poll failed");
        for (i = 0; i < 2; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                read_data(pfds[i].fd);
            }
            if (pfds[i].revents & (POLLHUP | POLLERR))
            {
                fprintf(stderr, "Client poll error\n");
                exit(1);
            }
        }
    }
    exit(0);
}

void read_data(int fd)
{
    int i, bytes_read;
    char buffer[256];
    bytes_read = read(fd, buffer, 256);
    if_error(bytes_read, "Unable to read buffer");
    if (fd == STDIN_FILENO)
    {
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\r':
            case '\n':
                exitcode = write(sfd, "\r\n", 2);
                if_error(exitcode, "Unable to write to socket\n");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write to stdout\n");
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\r':
            case '\n':
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write to stdout\n");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write to stdout\n");
                break;
            }
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

void parse_options(int argc, char **argv, int *port, char **log)
{
    int c;
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", optional_argument, NULL, 'l'},
        {"compress", no_argument, &compress_flag, 'c'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "plc", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 0:
            // set flag
            break;
        case 'p':
            *port = atoi(optarg);
            break;
        case 'l':
            *log = optarg;
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