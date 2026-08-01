/*
 * Resizable list using pointers to each element. Supports minimal operations.
 * The caller is responsible for allocating each element and providing a
 * function to free elements.
 */

#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list* List;

/*
 * Returns an empty List or NULL if an error occurs.
 */
List List_Create();

/*
 * Note: Does not free any elements in list.
 */
void List_Free(List list);

/*
 * Adds elem to the front of list. Returns false if elem couldn't be added.
 */
bool List_Push(List list, void *elem);

/*
 * Adds elem to list just after anchor. If anchor isn't in list, elem isn't
 * added.
 *
 * If the elements in list aren't unique, the first instance of anchor in list
 * is used.
 *
 * Returns false if elem couldn't be added for any reason.
 */
bool List_Insert(List list, void *anchor, void *elem);

size_t List_Length(List list);

/*
 * Applies func to every element in list.
 */
void List_ForEach(List list, void (*func)(void *elem));

/*
 * Removes every element from list where test returns true. Calls free for
 * each such element.
 */
void List_Filter(List list, bool (*test)(void *elem), void (*free)(void *elem));

/*
 * Returns true if any element in list returns true for test. May not call test
 * for every element.
 */
bool List_Check(List list, bool (*test)(void *elem));

#endif
