#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "SortedList.h"

int threads = 1, iterations = 1, lists_size = 1, opt_yield = 0, opt_sync = 0, exitcode, *s_locks;
// opt_yield: i=4, d=2, l=1
SortedList_t *lists;
SortedListElement_t *elements;
pthread_mutex_t *m_locks;

int hash_key(char *key);
long long lock(int hash);
void unlock(int hash);
void *thread_routine(void *ptr);
void print_tag();
void handle_sigsegv();
int is_valid_yield_opt(char *optarg);
void parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i, r, size = threads * iterations, nums[threads];
    long operations;
    long long completion_time = 0, lock_time = 0;
    struct timespec start, finish;
    pthread_t tids[threads];
    char *str;
    void **ptr;

    signal(SIGSEGV, handle_sigsegv);
    srand(time(0));

    // initialize empty list
    lists = (SortedList_t *)malloc(sizeof(SortedList_t) * lists_size);
    for (i = 0; i < lists_size; i++)
    {
        lists[i].next = &lists[i];
        lists[i].prev = &lists[i];
    }

    elements = (SortedListElement_t *)malloc(size * sizeof(SortedListElement_t));
    for (i = 0; i < size; i++)
    {
        elements[i] = *(SortedListElement_t *)malloc(sizeof(SortedListElement_t));
        r = rand() % 1000000;
        str = (char *)malloc(8 * sizeof(char));
        sprintf(str, "%d", r);
        elements[i].key = str;
        hash_key(str);
    }

    // initialize locks
    if (opt_sync == 'm')
    {
        m_locks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * lists_size);
        for (i = 0; i < lists_size; i++)
        {
            exitcode = pthread_mutex_init(&m_locks[i], NULL);
            if (exitcode != 0)
                fprintf(stderr, "Failed to initialize mutex\n");
        }
    }
    else if (opt_sync == 's')
    {
        s_locks = (int *)malloc(sizeof(int) * lists_size);
        memset(s_locks, 0, sizeof(int) * lists_size);
    }

    // start timer
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (i = 0; i < threads; i++)
    {
        nums[i] = i;
        pthread_create(&tids[i], NULL, thread_routine, &nums[i]);
    }

    ptr = (void *)malloc(sizeof(long long));
    for (i = 0; i < threads; i++)
    {
        pthread_join(tids[i], ptr);
        lock_time += (long long)*ptr;
    }
    free(ptr);

    // finish timer
    clock_gettime(CLOCK_MONOTONIC, &finish);

    if (opt_sync == 'm')
    {
        for (i = 0; i < lists_size; i++)
            pthread_mutex_destroy(&m_locks[i]);
    }

    for (i = 0; i < size; i++)
        free((void *)elements[i].key);
    free(elements);
    free(lists);

    operations = threads * iterations * 3;
    completion_time = (finish.tv_sec - start.tv_sec) * 1000000000 + (finish.tv_nsec - start.tv_nsec); // in nanoseconds

    print_tag(stdout);
    printf(",%d,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, lists_size, operations, completion_time, completion_time / operations, lock_time / operations);

    exit(0);
}

void *thread_routine(void *ptr)
{
    int i, hash = 0, t = *(int *)ptr, length;
    long long thread_time = 0;

    hash = hash_key((char *)&t);

    for (i = t * iterations; i < (t + 1) * iterations; i++)
    {
        thread_time += lock(hash);
        SortedList_insert(&lists[hash], &elements[i]);
        unlock(hash);
    }

    for (i = 0; i < lists_size; i++)
    {
        thread_time += lock(hash);
        length = SortedList_length(&lists[hash]);
        unlock(hash);
    }

    if (length < 0)
    {
        print_tag(stderr);
        fprintf(stderr, ": Invalid length\n");
        exit(1);
    }

    for (i = t * iterations; i < (t + 1) * iterations; i++)
    {
        thread_time += lock(hash);
        exitcode = SortedList_delete(SortedList_lookup(&lists[hash], elements[i].key));
        unlock(hash);

        if (exitcode)
        {
            print_tag(stderr);
            fprintf(stderr, ": Unable to find/delete element\n");
            exit(1);
        }
    }
    return (void *)thread_time;
}

int hash_key(char *key)
{
    return key[0] % lists_size;
}

long long lock(int hash)
{
    if (!(opt_sync == 'm' || opt_sync == 's'))
        return 0;
    struct timespec thread_start, thread_finish;
    clock_gettime(CLOCK_MONOTONIC, &thread_start);
    if (opt_sync == 'm')
        pthread_mutex_lock(&m_locks[hash]);
    else if (opt_sync == 's')
        while (__sync_lock_test_and_set(&s_locks[hash], 1))
            ;
    clock_gettime(CLOCK_MONOTONIC, &thread_finish);
    return (thread_finish.tv_sec - thread_start.tv_sec) * 1000000000 + (thread_finish.tv_nsec - thread_start.tv_nsec);
}

void unlock(int hash)
{
    if (opt_sync == 'm')
        pthread_mutex_unlock(&m_locks[hash]);
    else if (opt_sync == 's')
        __sync_lock_release(&s_locks[hash]);
}

void print_tag(FILE *stream)
{
    int temp;
    fprintf(stream, "list");
    if (opt_yield == 0)
        fprintf(stream, "-none");
    else
    {
        fprintf(stream, "-");
        temp = opt_yield;
        if (temp >= 4)
        {
            fprintf(stream, "i");
            temp -= 4;
        }
        if (temp >= 2)
        {
            fprintf(stream, "d");
            temp -= 2;
        }
        if (temp >= 1)
        {
            fprintf(stream, "l");
            temp -= 1;
        }
    }
    if (opt_sync == 'm' || opt_sync == 's')
        fprintf(stream, "-%c", opt_sync);
    else
        fprintf(stream, "-none");
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
        {"lists", required_argument, NULL, 'l'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "tiysl", long_options, NULL)) != -1)
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
                    opt_yield += 4;
                if (strchr(optarg, 'd') != NULL)
                    opt_yield += 2;
                if (strchr(optarg, 'l') != NULL)
                    opt_yield += 1;
            }
            else
            {
                fprintf(stderr, "Invalid yield option: %s\n", optarg);
                exit(1);
            }
            break;
        case 's':
            if (strcmp(optarg, "s") == 0 || strcmp(optarg, "m") == 0)
                opt_sync = optarg[0];
            else
            {
                fprintf(stderr, "Invalid sync option: %s\n", optarg);
                exit(1);
            }
            break;
        case 'l':
            lists_size = atoi(optarg);
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

void handle_sigsegv()
{
    print_tag(stderr);
    fprintf(stderr, ": Caught segmentation fault\n");
    exit(1);
}