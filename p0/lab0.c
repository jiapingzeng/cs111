// NAME: Jiaping Zeng
// EMAIL: jiapingzeng@ucla.edu
// ID: 905363270

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int segfault_flag;
static int catch_flag;

void parse_options(int argc, char **argv, char **input, char **output);
void handle_sigsegv();

int main(int argc, char **argv) {

    char *input = "";
    char *output = "";

    parse_options(argc, argv, &input, &output);
    
    int infd;
    int outfd;
    char ch;
    // create file with 644 permissions
    mode_t creat_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // redirect input/output based on arguments
    if (strcmp(input, "") == 0) infd = STDIN_FILENO;
    else {
        infd = open(input, O_RDONLY);
        if (infd < 0) {
            fprintf(stderr, "Unable to open input file '%s': %s\n", input, strerror(errno));
            exit(2);
        }
        close(STDIN_FILENO);
        dup2(infd, STDIN_FILENO);
        close(infd);
    }
    if (strcmp(output, "") == 0) outfd = STDOUT_FILENO;
    else {
        outfd = open(output, O_WRONLY | O_CREAT | O_TRUNC, creat_mode);
        if (outfd < 0) {
            fprintf(stderr, "Unable to open/create output file '%s': %s\n", output, strerror(errno));
            exit(3);
        }
        close(STDOUT_FILENO);
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
    }

    // register handler for segfault
    if (catch_flag) {
        signal(SIGSEGV, handle_sigsegv);
    }

    // force a segfault
    if (segfault_flag) {
        char *ptr = NULL;
        *ptr = 'a';
    }

    // copy input to output one character at a time
    while (read(STDIN_FILENO, &ch, 1) > 0) {
        write(STDOUT_FILENO, &ch, 1);
    }

    exit(0);
}

void parse_options(int argc, char **argv, char **input, char **output) {
    int c, option_index;
    static struct option long_options[] = {
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"segfault", no_argument, &segfault_flag, 1},
        {"catch", no_argument, &catch_flag, 1},
        {0, 0, 0, 0}
    };

    while (1) {
        c = getopt_long(argc, argv, "io", long_options, &option_index);
        if (c == -1) break; // end of options, exit while loop
        switch (c) {
            case 0:
                // set flag
                break;
            case 'i':
                *input = optarg;
                break;
            case 'o':
                *output = optarg;
                break;
            case '?':
                // getopt printed error
                fprintf(stderr, "Unable to retrieve option\n");
                exit(1);
            default:
                fprintf(stderr, "Unknown option: %d\n", c);
                exit(1);
        }
    }
}

void handle_sigsegv() {
    fprintf(stderr, "Caught segmentation fault\n");
    exit(4);
}