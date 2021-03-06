#include <string.h>
#include <stdio.h>
#include <sched.h>
#include "SortedList.h"

int debug = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if (!list || !element) {
        if (debug)
            printf("unable to insert: %p (prev: %p, next: %p, key: %s)\n", element, element->prev, element->next, element->key);
        return;
    }
    SortedListElement_t *ptr = list->next;
    while (ptr->key && strcmp(element->key, ptr->key) > 0)
        ptr = ptr->next;
    if (opt_yield & INSERT_YIELD)
        sched_yield();
    element->next = ptr;
    element->prev = ptr->prev;
    element->prev->next = element;
    element->next->prev = element;
    if (debug)
        printf("inserted 1 element: %s\n", element->key);
}

int SortedList_delete(SortedListElement_t *element)
{
    if (!element || !element->next || !element->prev || element->next->prev != element || element->prev->next != element)
    {
        if (debug)
            printf("unable to delete: %p (prev: %p, next: %p, key: %s)\n", element, element->prev, element->next, element->key);
        return 1;
    }
    if (opt_yield & DELETE_YIELD)
        sched_yield();
    element->next->prev = element->prev;
    element->prev->next = element->next;
    if (debug)
        printf("deleted 1 element: %s\n", element->key);
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    if (!list || !key)
        return NULL;
    SortedListElement_t *ptr = list->next;
    if (opt_yield & LOOKUP_YIELD)
        sched_yield();
    while (ptr->key)
    {
        if (strcmp(ptr->key, key) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
    if (!list)
        return -1;
    int count = 0;
    SortedListElement_t *ptr = list->next;
    if (opt_yield & LOOKUP_YIELD)
        sched_yield();
    while (ptr->key)
    {
        count++;
        ptr = ptr->next;
    }
    return count;
}