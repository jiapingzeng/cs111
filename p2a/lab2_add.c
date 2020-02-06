#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

int threads = 1, iterations = 1;

void add(long long *pointer, long long value);
void parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
    long long counter = 0;
    int i, total;
    struct timespec start, finish;
    pthread_t tid;

    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < threads; i++) {
        pthread_create(&tid, NULL, NULL, NULL);
    }
    pthread_exit(NULL);

    clock_gettime(CLOCK_REALTIME, &finish);
    operations = threads * iterations * 2;
    long long time = (finish.tv_sec - start.tv_sec) * 1000000000 + (finish.tv_nsec - start.tv_nsec); // in nanoseconds
    printf("add-none,%d,%d,%lld,%lld,%lld,%d", threads, iterations, operations, time, time / operations, total);

    exit(0);
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "ti", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 't':
            threads = atoi(optarg);
            break;
        case 'i':
            iterations = atoi(optarg);
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

void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    *pointer = sum;
}
