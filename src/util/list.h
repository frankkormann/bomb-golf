/*
 * Array-based list supporting the minimum operations.
 */

#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list *List;

/*
 * Returns a new List or NULL if there isn't enough memory.
 */
List List_Create();

/*
 * Note: Does not free any elements in list.
 */
void List_Free(List list);

bool List_Append(List list, void *elem);

/*
 * If list is empty, returns NULL.
 */
void* List_Last(List list);

void List_ForEach(List list, void (*func)(void *elem));

#endif
