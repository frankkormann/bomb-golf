#include <malloc.h>
#include <stddef.h>
#include <stdbool.h>
#include "queue.h"

typedef struct node Node;
struct node {
	void *val;
	Node *next;
};

struct queue {
	Node *head;
	Node *tail;
};

Queue Queue_Create() {
	Queue q = malloc(sizeof(*q));
	if (!q) return NULL;

	q->head = NULL;
	q->tail = NULL;

	return q;
}

void Queue_Free(Queue q) {
	Node *cur = q->head;
	while (cur) {
		Node *next = cur->next;
		free(cur);
		cur = next;
	}
	free(q);
}

bool Queue_Push(Queue q, void *elem) {
	Node *new = malloc(sizeof(*new));
	if (!new) return false;

	new->val = elem;
	new->next = NULL;
	if (q->tail) q->tail->next = new;
	q->tail = new;
	if (!q->head) q->head = new;

	return true;
}

void* Queue_Pop(Queue q) {
	if (!q->head) return NULL;

	Node *oldHead = q->head;
	void *rv = oldHead->val;
	q->head = q->head->next;
	if (!q->head) q->tail = NULL;

	free(oldHead);
	return rv;
}

bool Queue_IsEmpty(Queue q) {
	return !q->head;
}
