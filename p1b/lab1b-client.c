// NAME: Jiaping Zeng
// EMAIL: jiapingzeng@ucla.edu
// ID: 905363270

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
#include <sys/stat.h>
#include <fcntl.h>
#include "zlib.h"

#define BUFFER_SIZE 256

int port, exitcode, sfd, logfd;
static int log_flag, compress_flag;
struct termios oldconfig;
z_stream def_stream, inf_stream;

void read_data(int fd);
void parse_options(int argc, char **argv, int *port, char **log);
void if_error(int error, char *message);
void initialize();
void terminate();
int inf(char *src, char *dest, int src_size, int dest_size);
int def(char *src, char *dest, int src_size, int dest_size);

int main(int argc, char **argv)
{
    char *log_file;
    int i;

    parse_options(argc, argv, &port, &log_file);
    initialize();
    atexit(terminate);

    // set up polls
    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[1].fd = sfd;
    pfds[0].events = POLLIN | POLLHUP | POLLERR;
    pfds[1].events = POLLIN | POLLHUP | POLLERR;

    // set up logging
    if (log_flag)
    {
        mode_t creat_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 644
        logfd = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, creat_mode);
        if_error(logfd, "Unable to open/create log file");
    }

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
                fprintf(stderr, "Poll error\n");
                exit(1);
            }
        }
    }
    exit(0);
}

void read_data(int fd)
{
    int i, bytes_read, bytes_compressed;
    char buffer[BUFFER_SIZE], cbuffer[BUFFER_SIZE];
    bytes_read = read(fd, buffer, BUFFER_SIZE);
    if_error(bytes_read, "Unable to read buffer");
    if (fd == STDIN_FILENO)
    {
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\r':
            case '\n':
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write CRLF to stdout");
                if (compress_flag)
                {
                    bytes_compressed = def("\n", cbuffer, 1, BUFFER_SIZE);
                    write(sfd, &cbuffer, bytes_compressed);
                }
                else
                {
                    exitcode = write(sfd, "\n", 1);
                    if_error(exitcode, "Unable to write LF to socket");
                }
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write to stdout");
                if (compress_flag)
                {
                    bytes_compressed = def(&buffer[i], cbuffer, 1, BUFFER_SIZE);
                    write(sfd, &cbuffer, bytes_compressed);
                }
                else
                {
                    exitcode = write(sfd, &buffer[i], 1);
                    if_error(exitcode, "Unable to write to socket");
                }
                break;
            }
        }
        if (log_flag)
        {
            if (compress_flag)
            {
                dprintf(logfd, "SENT %d bytes: ", bytes_compressed);
                write(logfd, &cbuffer, bytes_compressed);
            }
            else
            {
                dprintf(logfd, "SENT %d bytes: ", bytes_read);
                write(logfd, &buffer, bytes_read);
            }
            write(logfd, "\n", 1);
        }
    }
    else
    {
        if (bytes_read == 0)
        {
            exit(1);
        }
        if (log_flag)
        {
            dprintf(logfd, "RECEIVED %d bytes: ", bytes_read);
            write(logfd, &buffer, bytes_read);
            write(logfd, "\n", 1);
        }
        if (compress_flag)
        {
            memcpy(cbuffer, buffer, bytes_read);
            bytes_read = inf(cbuffer, buffer, bytes_read, BUFFER_SIZE);
        }
        for (i = 0; i < bytes_read; i++)
        {
            switch (buffer[i])
            {
            case '\r':
            case '\n':
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write to stdout");
                break;
            default:
                exitcode = write(STDOUT_FILENO, &buffer[i], 1);
                if_error(exitcode, "Unable to write to stdout");
                break;
            }
        }
    }
}

void initialize()
{
    // set up input mode
    struct termios newconfig;
    exitcode = tcgetattr(STDIN_FILENO, &oldconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    exitcode = tcgetattr(STDIN_FILENO, &newconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);
    if_error(exitcode, "Unable to set terminal parameters");

    // set up socket
    int len;
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
}

void terminate()
{
    // reset input mode
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
    if_error(exitcode, "Unable to set terminal parameters");

    // deflate zstreams
    if (compress_flag)
    {
        deflateEnd(&def_stream);
        deflateEnd(&inf_stream);
    }
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
            log_flag = 1;
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