/*
 * Ensures that an event is not handled more than once.
 *
 * An event is simply a signal that an event has occurred. A new dispatcher
 * should be created for each event type.
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdbool.h>

typedef struct dispatcher *Dispatcher;

typedef struct {
	/*
	 * Handlers with higher priority will receive events first.
	 */
	int priority;
	/*
	 * Passed to handle.
	 */
	void *handleParam;
	/*
	 * Function to be called when an event needs to be handled. Returns
	 * false if the event should continue to be propagated.
	 */
	bool (*handle)(void *param);
} Dispatcher_Handler;

/*
 * Returns NULL if an error occurs.
 */
Dispatcher Dispatcher_Create();

void Dispatcher_Free(Dispatcher dispatcher);

/*
 * Handlers with a higher priority will receive events before those with a
 * lower priority. In case of ties, the handler wins that was registered first.
 *
 * Multiple handlers with the same handleParam and handle function should not
 * be registered.
 *
 * Returns false if an error occurs.
 */
bool Dispatcher_AddHandler(Dispatcher dispatcher, Dispatcher_Handler handler);

/*
 * Removes the registered handler which matches handler except for priority.
 */
void Dispatcher_RemoveHandler(Dispatcher dispatcher, Dispatcher_Handler handler);

/*
 * Sends the event to each handler in order until one handles it.
 */
void Dispatcher_DispatchEvent(Dispatcher dispatcher);

#endif
