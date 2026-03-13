/*
 * Ensures that an event is not handled more than once.
 *
 * An event has one argument, a void*. To avoid misinterpretation, a new
 * dispatcher should be  created for each event type.
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdbool.h>

typedef struct dispatcher *Dispatcher;

typedef struct {
	int priority;
	/*
	 * Function to be called when an event needs to be handled. Returns
	 * true if the event was handled.
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
 * Returns false if an error occurs.
 */
bool Dispatcher_AddHandler(Dispatcher dispatcher, Dispatcher_Handler handler);

/*
 * Sends the event to each handler in order until one handles it.
 */
void Dispatcher_DispatchEvent(Dispatcher dispatcher, void *param);

#endif
