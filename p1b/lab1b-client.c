#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define PORT 3000

static int port;
int exitcode;

void parse_options(int argc, char **argv);
void if_error(int error, char *message);

int main(int argc, char **argv) {

    parse_options(argc, argv);

    int sfd, bytes_read;
    unsigned int len;
    struct sockaddr_in addr;
    char buffer[1024];
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if_error(sfd, "Unable to open socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    len = sizeof(addr);

    exitcode = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if_error(exitcode, "Address not supported");

    exitcode = connect(sfd, (struct sockaddr *)&addr, (socklen_t)len);
    if_error(exitcode, "Connection failed");

    char *test = "Client test";
    send(sfd, test, strlen(test), 0);

    printf("Client sent test\n");

    bytes_read = read(sfd, buffer, 1024);
    printf("%s\n", buffer);

    exit(0);
}

void parse_options(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "p", long_options, NULL)) != -1) {
        switch (c) {
            case 0:
                // set flag
                break;
            case 'p':
                port = atoi(optarg);
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

void if_error(int error, char *message) {
    if (error < 0) {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}