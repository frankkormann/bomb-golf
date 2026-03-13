#include <stdbool.h>
#include <malloc.h>
#include "dispatcher.h"

typedef struct list_node *ListNode;
struct list_node {
	ListNode next;
	Dispatcher_Handler handler;
};

struct dispatcher {
	ListNode firstHandler;
};

Dispatcher Dispatcher_Create() {
	Dispatcher dispatcher = malloc(sizeof(struct dispatcher));
	if (!dispatcher) return NULL;

	dispatcher->firstHandler = NULL;

	return dispatcher;
}

void Dispatcher_Free(Dispatcher dispatcher) {
	ListNode node = dispatcher->firstHandler;
	while (node != NULL) {
		ListNode next = node->next;
		free(node);
		node = next;
	}
	free(dispatcher);
}

bool Dispatcher_AddHandler(Dispatcher dispatcher, Dispatcher_Handler newHandler) {
	ListNode new = malloc(sizeof(*new));
	if (!new) return NULL;

	new->handler = newHandler;

	ListNode *pointerToCurrent = &dispatcher->firstHandler;
	while (*pointerToCurrent != NULL) {
		if ((*pointerToCurrent)->handler.priority < newHandler.priority) {
			break;
		}
		pointerToCurrent = &(*pointerToCurrent)->next;
	}

	new->next = *pointerToCurrent;
	*pointerToCurrent = new;

	return true;
}

void Dispatcher_DispatchEvent(Dispatcher dispatcher, void *param) {
	// Handlers are stored in priority order
	for (ListNode node = dispatcher->firstHandler; node; node = node->next) {
		if (node->handler.handle(param)) break;
	}
}
