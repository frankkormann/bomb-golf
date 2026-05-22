/*
 * Resizable queue using pointers to each element. The caller is responsible for
 * allocating and freeing each pointer.
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

typedef struct queue* Queue;

/*
 * Returns a new Queue or NULL if there isn't enough memory.
 */
Queue Queue_Create();

/*
 * Note: Does not free any elements in q.
 */
void Queue_Free(Queue q);

/*
 * Adds elem to the end of q.
 *
 * Returns false if q could not be resized to fit elem.
 */
bool Queue_Push(Queue q, void *elem);

/*
 * Removes the top element from q and returns it. Returns NULL if q is empty.
 */
void* Queue_Pop(Queue q);

/*
 * Returns true if there are no elements in q.
 */
bool Queue_IsEmpty(Queue q);

#endif
