#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int segfault_flag;
static int catch_flag;

void parse_options(int argc, char **argv, char **input, char **output);

int main(int argc, char **argv) {

    char *input = "";
    char *output = "";

    parse_options(argc, argv, &input, &output);
    
    printf("-----\nParsed options:\n");
    printf("  in: %s\n", strcmp(input, "") == 0 ? "(empty)" : input);
    printf("  out: %s\n", strcmp(output, "") == 0 ? "(empty)" : output);
    printf("  segfault: %d\n", segfault_flag);
    printf("  catch: %d\n", catch_flag);
    printf("-----\n");

    // creates/opens file if output is not specified
    if (strcmp(output, "") != 0) {
        int fd;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        fd = open(output, O_WRONLY | O_CREAT, mode);
        printf("%d", fd);
    }

    if (strcmp(input, "") == 0) {
        // read from stdin
        if (strcmp(output, "") == 0) {
            // write to stdout
            char ch;
            while (read(STDIN_FILENO, &ch, 1) > 0) {
                printf("%c", ch);
            }
        } else {
            // write to specified output
        }
    } else {
        // read from specified input
        if (strcmp(output, "") == 0) {
            // write to stdout
        } else {
            // write to specified output
            
        }
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