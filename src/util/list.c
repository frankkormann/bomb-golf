#include <malloc.h>
#include "list.h"

#define MINIMUM_ARR_SIZE 1
#define ARR_RESIZE_MULTIPLIER 2

struct list {
	void **arr;
	size_t size;
	size_t capacity;
};

List List_Create() {
	List list = malloc(sizeof(*list));
	if (!list) return NULL;

	list->arr = malloc(sizeof(void*) * MINIMUM_ARR_SIZE);
	if (!list->arr) {
		free(list);
		return NULL;
	}

	list->size = 0;
	list->capacity = MINIMUM_ARR_SIZE;

	return list;
}

void List_Free(List list) {
	free(list->arr);
	free(list);
}

bool List_Append(List list, void *elem) {
	if (list->size >= list->capacity) {
		void **newArr = realloc(list->arr, sizeof(void*) * list->capacity
				* ARR_RESIZE_MULTIPLIER);
		if (!newArr) return false;
		list->arr = newArr;
		list->capacity *= ARR_RESIZE_MULTIPLIER;
	}
	list->arr[list->size] = elem;
	list->size++;
	return true;
}

void* List_Last(List list) {
	return list->size > 0 ? list->arr[list->size - 1] : NULL;
} 

void List_ForEach(List list, void (*func)(void *elem)) {
	for (size_t i = 0; i < list->size; i++) {
		func(list->arr[i]);
	}
}
