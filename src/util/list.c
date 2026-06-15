#include <malloc.h>
#include <stdbool.h>
#include "list.h"

typedef struct node Node;
struct node {
	void *val;
	Node *next;
};

struct list {
	Node *head;
};

List List_Create() {
	List list = malloc(sizeof(*list));
	if (!list) return NULL;

	list->head = NULL;

	return list;
}

void List_Free(List list) {
	Node *n = list->head;
	while (n) {
		Node *next = n->next;
		free(n);
		n = next;
	}
	free(list);
}

bool List_Push(List list, void *elem) {
	Node *new = malloc(sizeof(*new));
	if (!new) return false;

	new->val = elem;
	new->next = list->head;
	list->head = new;

	return true;
}

void List_ForEach(List list, void (*func)(void *elem)) {
	for (Node *n = list->head; n; n = n->next) {
		func(n->val);
	}
}

void List_Filter(List list, bool (*test)(void *elem), void (*argFree)(void *elem)) {
	Node **prev = &list->head;
	Node *n = list->head;
	while (n) {
		Node *next = n->next;
		if (test(n->val)) {
			*prev = next;
			argFree(n->val);
			free(n);
		} else {
			prev = &n->next;
		}
		n = next;
	}
}
