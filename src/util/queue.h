/*
 * Resizable queue using pointers to each element. The caller is responsible for
 * allocating and freeing each pointer.
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

typedef struct queue* Queue;

/*
 * Returns the Queue or NULL if there isn't enough memory.
 */
Queue Queue_Create();

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
 * Identical to Queue_Pop except there is no check to reduce the internal size
 * of q. Useful for popping many elements in a row or preventing a resize after
 * using Queue_EnsureCapacity.
 */
void* Queue_FastPop(Queue q);

/*
 * Attempts to increase q's internal size to fit minCapacity elements.
 *
 * Returns true if the resize was successful.
 */
bool Queue_EnsureCapacity(Queue q, size_t minCapacity);

/*
 * Performs a best attempt at reducing the memory used by q.
 */
void Queue_Prune(Queue q);

/*
 * Returns true if there are no elements in q.
 */
bool Queue_IsEmpty(Queue q);

#endif
