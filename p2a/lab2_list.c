#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "SortedList.h"

int threads = 1, iterations = 1, yield_opt = 0, sync_opt = 0;
// yield_opt: i=4, d=2, l=1

void print_tag();
void *thread_routine(void *ptr);
int is_valid_yield_opt(char *optarg);
void parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i, counter;
    long operations;
    long long time;
    struct timespec start, finish;
    pthread_t tids[threads];

    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < threads; i++)
    {
        pthread_create(&tids[i], NULL, thread_routine, &counter);
    }

    for (i = 0; i < threads; i++)
    {
        pthread_join(tids[i], NULL);
    }

    clock_gettime(CLOCK_REALTIME, &finish);

    operations = threads * iterations * 3;
    time = (finish.tv_sec - start.tv_sec) * 1000000000 + (finish.tv_nsec - start.tv_nsec); // in nanoseconds
    
    print_tag();
    printf(",%d,%d,%d,%ld,%lld,%lld\n", threads, iterations, 1, operations, time, time/operations);

    exit(0);
}

void print_tag()
{
    int temp;
    printf("list");
    if (yield_opt == 0)
        printf("-none");
    else
    {
        printf("-");
        temp = yield_opt;
        if (temp >= 4)
        {
            printf("i");
            temp -= 4;
        }
        if (temp >= 2)
        {
            printf("d");
            temp -= 2;
        }
        if (temp >= 1)
        {
            printf("l");
            temp -= 1;
        }
    }
    if (sync_opt == 'm' || sync_opt == 's')
        printf("-%c", sync_opt);
    else
        printf("-none");
}

void *thread_routine(void *ptr)
{
    return ptr;
}

int is_valid_yield_opt(char *optarg)
{
    if (strlen(optarg) > 3)
        return 0;
    unsigned int i;
    for (i = 0; i < strlen(optarg); i++)
    {
        if (strchr("idl", optarg[i]) == NULL)
            return 0;
    }
    return 1;
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "tiys", long_options, NULL)) != -1)
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
            if (is_valid_yield_opt(optarg))
            {
                if (strchr(optarg, 'i') != NULL)
                    yield_opt += 4;
                if (strchr(optarg, 'd') != NULL)
                    yield_opt += 2;
                if (strchr(optarg, 'l') != NULL)
                    yield_opt += 1;
            }
            else
            {
                fprintf(stderr, "Invalid yield option: %s\n", optarg);
                exit(1);
            }
            break;
        case 's':
            if (strcmp(optarg, "s") == 0 || strcmp(optarg, "m") == 0)
                sync_opt = optarg[0];
            else
            {
                fprintf(stderr, "Invalid sync option: %s\n", optarg);
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