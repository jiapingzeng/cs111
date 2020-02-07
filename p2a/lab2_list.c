#include <getopt.h>

int yield_opt = 0;

void parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "tiy", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 't':
            threads = atoi(optarg);
            break;
        case 'i':
            iterations = atoi(optarg);
            break;
        case 'y':
            if (strcmp(optarg, "i") == 0 || strcmp(optarg, "d") == 0 || strcmp(optarg, "l") == 0)
                yield_opt = optarg[0];
            else
            {
                fprintf(stderr, "Invalid yield option: %s\n", optarg);
                exit(1);
            }
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