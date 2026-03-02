#include <malloc.h>
#include <stddef.h>
#include <stdbool.h>
#include "queue.h"

// If the proportion of unused array space dips below this, a reallocation
// and rebalance is triggered
#define REBALANCE_THRESHOLD 0.7

// Proportion of empty space to keep after rebalancing
#define BUFFER_PROPORTION 0.2

#define MINIMUM_ARR_SIZE 1

struct queue {
	size_t front;
	size_t back;
	void **arr;
	size_t arrSize;
};

Queue Queue_Create() {
	Queue q = malloc(sizeof(*q));
	if (!q) return NULL;

	q->arr = malloc(sizeof(void*) * MINIMUM_ARR_SIZE);
	if (!q->arr) {
		free(q);
		return NULL;
	}

	q->arrSize = MINIMUM_ARR_SIZE;
	q->front = 0;
	q->back = 0;

	return q;
}

void Queue_Free(Queue q) {
	free(q->arr);
	free(q);
}

static void rebalance(Queue q) {
	size_t capacity = q->front - q->back;
	if (q->back > 0) {
		for (size_t i = 0, j = q->back; i < capacity; i++, j++) {
			q->arr[i] = q->arr[j];
		}
	}

	q->back = 0;
	q->front = capacity;

	size_t newArrSize = capacity * (1 + BUFFER_PROPORTION);
	if (newArrSize == 0) newArrSize = MINIMUM_ARR_SIZE;
	if (newArrSize <= q->arrSize) newArrSize = q->arrSize + 1;

	void **newArr = realloc(q->arr, sizeof(void*) * newArrSize);
	if (newArr) {
		q->arr = newArr;
		q->arrSize = newArrSize;
	}
}

bool Queue_Push(Queue q, void *elem) {
	if (q->front >= q->arrSize) {
		rebalance(q);
		if (q->front >= q->arrSize) return false;
	}

	q->arr[q->front] = elem;
	q->front++;
	return true;
}

void* Queue_Pop(Queue q) {
	void *elem = Queue_FastPop(q);
	if (q->front - q->back < REBALANCE_THRESHOLD * q->arrSize) {
		rebalance(q);
	}
	return elem;
}

void* Queue_FastPop(Queue q) {
	if (Queue_IsEmpty(q)) return NULL;

	void *elem = q->arr[q->back];
	q->back++;
	return elem;
}

void Queue_Prune(Queue q) {
	rebalance(q);
}

bool Queue_IsEmpty(Queue q) {
	return q->front == q->back;
}
