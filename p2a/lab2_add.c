#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

int threads = 1, iterations = 1, opt_sync = 0, exitcode;
static int opt_yield;
pthread_mutex_t m_lock;

void print_tag();
void add(long long *pointer, long long value);
void add_none(long long *pointer, long long value);
void add_m(long long *pointer, long long value);
void add_s(long long *pointer, long long value);
void add_c(long long *pointer, long long value);
void *thread_routine(void *ptr);
void parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i;
    long operations;
    long long counter = 0, completion_time;
    struct timespec start, finish;
    pthread_t tids[threads];

    if (opt_sync == 'm')
    {
        exitcode = pthread_mutex_init(&m_lock, NULL);
        if (exitcode != 0)
            fprintf(stderr, "Failed to initialize mutex\n");
    }

    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < threads; i++)
        pthread_create(&tids[i], NULL, thread_routine, &counter);

    for (i = 0; i < threads; i++)
        pthread_join(tids[i], NULL);

    clock_gettime(CLOCK_REALTIME, &finish);

    if (opt_sync == 'm')
        pthread_mutex_destroy(&m_lock);

    operations = threads * iterations * 2;
    completion_time = (finish.tv_sec - start.tv_sec) * 1000000000 + (finish.tv_nsec - start.tv_nsec); // in nanoseconds

    print_tag();
    printf(",%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, operations, completion_time, completion_time / operations, counter);

    exit(0);
}

void print_tag()
{
    printf("add");
    if (opt_yield)
        printf("-yield");
    if (opt_sync == 'm' || opt_sync == 's' || opt_sync == 'c')
        printf("-%c", opt_sync);
    else
        printf("-none");
}

void *thread_routine(void *ptr)
{
    int i;
    for (i = 0; i < iterations; i++)
    {
        add((long long *)ptr, 1);
    }
    for (i = 0; i < iterations; i++)
    {
        add((long long *)ptr, -1);
    }
    return NULL;
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, &opt_yield, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "tiys", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 0:
            break;
        case 't':
            threads = atoi(optarg);
            break;
        case 'i':
            iterations = atoi(optarg);
            break;
        case 's':
            if (strcmp(optarg, "m") == 0 || strcmp(optarg, "s") == 0 || strcmp(optarg, "c") == 0)
                opt_sync = optarg[0];
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

void add(long long *pointer, long long value)
{
    if (opt_sync == 'm')
        add_m(pointer, value);
    else if (opt_sync == 's')
        add_s(pointer, value);
    else if (opt_sync == 'c')
        add_c(pointer, value);
    else
        add_none(pointer, value);
}

void add_none(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void add_m(long long *pointer, long long value)
{
    pthread_mutex_lock(&m_lock);
    add_none(pointer, value);
    pthread_mutex_unlock(&m_lock);
}

void add_s(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    while (__sync_lock_test_and_set(pointer, sum))
        ;
    if (opt_yield)
        sched_yield();
    __sync_lock_release(pointer);
}

void add_c(long long *pointer, long long value)
{
    long long old, new;
    do
    {
        old = *pointer;
        new = old + value;
        if (opt_yield)
            sched_yield();
    } while (__sync_val_compare_and_swap(pointer, old, new) != old);
}
