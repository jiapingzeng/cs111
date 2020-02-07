#include <string.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if (!list || !element)
        return;
    SortedListElement_t *ptr = list->next;
    while (ptr->key != NULL && strcmp(element->key, ptr->key) < 0)
        ptr = ptr->next;
    element->next = ptr;
    element->prev = ptr->prev;
    element->prev->next = element;
    element->next->prev = element;
}

int SortedList_delete(SortedListElement_t *element)
{
    if (!element)
        return 1;
    if (element->next->prev != element || element->prev->next != element)
        return 1;
    element->next->prev = element.prev;
    element->prev->next = element.next;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    if (!list)
        return NULL;
    SortedListElement_t *ptr = list->next;
    while (ptr->key != NULL)
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
    while (ptr->key != NULL)
    {
        count++;
        ptr = ptr->next;
    }
    return count;
}