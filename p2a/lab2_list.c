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

int threads = 1, iterations = 1, opt_yield = 0, opt_sync = 0, exitcode, s_lock = 0;
// opt_yield: i=4, d=2, l=1
SortedList_t *list;
SortedListElement_t *elements;
pthread_mutex_t m_lock;

void *thread_routine(void *ptr);
void print_tag();
void handle_sigsegv();
int is_valid_yield_opt(char *optarg);
void parse_options(int argc, char **argv);
void print_list(SortedList_t *list);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i, r, size = threads * iterations;
    long operations;
    long long completion_time;
    struct timespec start, finish;
    pthread_t tids[threads];
    char *str;

    signal(SIGSEGV, handle_sigsegv);
    srand(time(0));

    // initialize empty list
    list = (SortedList_t *)malloc(sizeof(SortedList_t));
    list->next = list;
    list->prev = list;

    elements = (SortedListElement_t *)malloc(size * sizeof(SortedListElement_t));
    for (i = 0; i < size; i++)
    {
        elements[i] = *(SortedListElement_t *)malloc(sizeof(SortedListElement_t));
        r = rand() % 1000000;
        str = (char *)malloc(8 * sizeof(char));
        sprintf(str, "%d", r);
        elements[i].key = str;
    }

    // for (i = 0; i < size; i++) SortedList_insert(list, &elements[i]);

    if (opt_sync == 'm')
    {
        exitcode = pthread_mutex_init(&m_lock, NULL);
        if (exitcode != 0)
            fprintf(stderr, "Failed to initialize mutex\n");
    }

    // start timer
    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < threads; i++)
        pthread_create(&tids[i], NULL, thread_routine, &i);

    for (i = 0; i < threads; i++)
        pthread_join(tids[i], NULL);

    // finish timer
    clock_gettime(CLOCK_REALTIME, &finish);

    //print_list(list);

    if (opt_sync == 'm')
        pthread_mutex_destroy(&m_lock);

    for (i = 0; i < size; i++)
        free((void *)elements[i].key);
    free(elements);
    free(list);

    operations = threads * iterations * 3;
    completion_time = (finish.tv_sec - start.tv_sec) * 1000000000 + (finish.tv_nsec - start.tv_nsec); // in nanoseconds

    print_tag();
    printf(",%d,%d,%d,%ld,%lld,%lld\n", threads, iterations, 1, operations, completion_time, completion_time / operations);

    exit(0);
}

void *thread_routine(void *ptr)
{
    int i, t = *(int *)ptr;

    // insert
    for (i = t * iterations; i < (t + 1) * iterations; i++)
    {
        if (opt_sync == 'm')
        {
            pthread_mutex_lock(&m_lock);
            SortedList_insert(list, &elements[i]);
            pthread_mutex_unlock(&m_lock);
        }
        else if (opt_sync == 's')
        {
            while (__sync_lock_test_and_set(&s_lock, 1))
                ;
            SortedList_insert(list, &elements[i]);
            __sync_lock_release(&s_lock);
        }
        else
            SortedList_insert(list, &elements[i]);
    }

    // get length
    int length;
    if (opt_sync == 'm')
    {
        pthread_mutex_lock(&m_lock);
        length = SortedList_length(list);
        pthread_mutex_unlock(&m_lock);
    }
    else if (opt_sync == 's')
    {
        while (__sync_lock_test_and_set(&s_lock, 1))
            ;
        length = SortedList_length(list);
        __sync_lock_release(&s_lock);
    }
    else
        length = SortedList_length(list);
    
    if (length < 0)
    {
        fprintf(stderr, "Invalid length\n");
        exit(1);
    }

    // look up and delete
    for (i = t * iterations; i < (t + 1) * iterations; i++)
    {
        SortedListElement_t *element;

        // look up
        if (opt_sync == 'm')
        {
            pthread_mutex_lock(&m_lock);
            element = SortedList_lookup(list, elements[i].key);
            pthread_mutex_unlock(&m_lock);
        }
        else if (opt_sync == 's')
        {
            while (__sync_lock_test_and_set(&s_lock, 1))
                ;
            element = SortedList_lookup(list, elements[i].key);
            __sync_lock_release(&s_lock);
        }
        else
            element = SortedList_lookup(list, elements[i].key);

        if (!elements)
        {
            fprintf(stderr, "Element not found\n");
            exit(1);
        }

        // delete
        if (opt_sync == 'm')
        {
            pthread_mutex_lock(&m_lock);
            exitcode = SortedList_delete(element);
            pthread_mutex_unlock(&m_lock);
        }
        else if (opt_sync == 's')
        {
            while (__sync_lock_test_and_set(&s_lock, 1))
                ;
            exitcode = SortedList_delete(element);
            __sync_lock_release(&s_lock);
        }
        else
            exitcode = SortedList_delete(element);

        if (exitcode)
        {
            fprintf(stderr, "Unable to delete element\n");
            exit(1);
        }
    }
    return NULL;
}

void print_tag()
{
    int temp;
    printf("list");
    if (opt_yield == 0)
        printf("-none");
    else
    {
        printf("-");
        temp = opt_yield;
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
    if (opt_sync == 'm' || opt_sync == 's')
        printf("-%c", opt_sync);
    else
        printf("-none");
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

void print_list(SortedList_t *list)
{
    if (!list)
        return;
    printf("{ ");
    SortedListElement_t *ptr = list->next;
    while (ptr->key != NULL)
    {
        printf("%s ", ptr->key);
        ptr = ptr->next;
    }
    printf("}, length: %d\n", SortedList_length(list));
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
    fprintf(stderr, "Caught segmentation fault\n");
    exit(1);
}