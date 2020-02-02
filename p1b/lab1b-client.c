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

int exitcode, sfd, logfd;
static int log_flag, compress_flag;
struct termios oldconfig;
z_stream def_stream, inf_stream;

void read_data(int fd);
void set_input_mode();
void exit_cleanup();
void parse_options(int argc, char **argv, int *port, char **log);
void if_error(int error, char *message);
int inf(char *src, char *dest, int src_size);
int def(char *src, char *dest, int src_size);

int main(int argc, char **argv)
{
    int port;
    char *log_file;

    parse_options(argc, argv, &port, &log_file);
    set_input_mode();
    atexit(exit_cleanup);

    unsigned int i;
    unsigned int len;
    struct sockaddr_in addr;

    // set up socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if_error(sfd, "Unable to open socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    len = sizeof(addr);

    exitcode = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if_error(exitcode, "Address not supported");

    exitcode = connect(sfd, (struct sockaddr *)&addr, (socklen_t)len);
    if_error(exitcode, "Connection failed");

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
    unsigned int i, bytes_read, bytes_compressed;
    char buffer[256], compressed[256];
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
                exitcode = write(STDOUT_FILENO, "\r\n", 2);
                if_error(exitcode, "Unable to write CRLF to stdout");
                if (compress_flag)
                {
                    def_stream.avail_in = (uInt)strlen("\n") + 1;
                    def_stream.next_in = (Bytef *)"\n";
                    def_stream.avail_out = (uInt)sizeof(compressed);
                    def_stream.next_out = (Bytef *)compressed;
                    deflate(&def_stream, Z_SYNC_FLUSH);
                    bytes_compressed = strlen(compressed);
                    write(sfd, compressed, bytes_compressed);
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
                    def_stream.avail_in = (uInt)strlen(&buffer[i]) + 1;
                    def_stream.next_in = (Bytef *)&buffer[i];
                    def_stream.avail_out = (uInt)sizeof(compressed);
                    def_stream.next_out = (Bytef *)compressed;
                    deflate(&def_stream, Z_SYNC_FLUSH);
                    bytes_compressed = strlen(compressed);
                    write(sfd, compressed, bytes_compressed);
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
                write(logfd, &compressed, bytes_compressed);
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
        if (compress_flag)
        {
            memcpy(compressed, buffer, bytes_read);
            inf_stream.avail_in = (uInt)((char *)def_stream.next_out - compressed);
            inf_stream.next_in = (Bytef *)compressed;
            inf_stream.avail_out = (uInt)sizeof(buffer);
            inf_stream.next_out = (Bytef *)buffer;
            inflate(&inf_stream, Z_SYNC_FLUSH);
        }
        if (log_flag)
        {
            dprintf(logfd, "RECEIVED %d bytes: ", bytes_read);
            write(logfd, &buffer, bytes_read);
            write(logfd, "\n", 1);
        }
        for (i = 0; i < strlen(buffer); i++)
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

void set_input_mode()
{
    struct termios newconfig;
    // set input mode
    exitcode = tcgetattr(STDIN_FILENO, &oldconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    exitcode = tcgetattr(STDIN_FILENO, &newconfig);
    if_error(exitcode, "Unable to get terminal parameters");
    newconfig.c_iflag = ISTRIP;
    newconfig.c_oflag = 0;
    newconfig.c_lflag = 0;
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &newconfig);
    if_error(exitcode, "Unable to set terminal parameters");
}

void exit_cleanup()
{
    // reset input mode
    exitcode = tcsetattr(STDIN_FILENO, TCSANOW, &oldconfig);
    if_error(exitcode, "Unable to set terminal parameters");

    // deflate zstreams
    if (compress_flag)
    {
        exitcode = deflateEnd(&def_stream);
        if_error(exitcode, "Deflate forward stream failed");
        exitcode = deflateEnd(&inf_stream);
        if_error(exitcode, "Deflate backward stream failed");
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

int def(char *src, char *dest, int src_size)
{
    def_stream.avail_in = (uInt)src_size;
    def_stream.next_in = (Bytef *)dest;
    def_stream.avail_out = (uInt)sizeof(dest);
    def_stream.next_out = (Bytef *)src;

    exitcode = deflate(&def_stream, Z_SYNC_FLUSH);
    if_error(exitcode, "Deflation failed");

    return sizeof(dest) - def_stream.avail_out;
}

int inf(char *src, char *dest, int src_size)
{
    inf_stream.avail_in = (uInt)src_size;
    inf_stream.next_in = (Bytef *)dest;
    inf_stream.avail_out = (uInt)sizeof(dest);
    inf_stream.next_out = (Bytef *)src;

    exitcode = inflate(&inf_stream, Z_SYNC_FLUSH);
    if_error(exitcode, "Inflation failed");

    return sizeof(dest) - inf_stream.avail_out;
}