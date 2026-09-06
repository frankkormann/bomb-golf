/*
 * Scene to display an error message.
 */

#ifndef ERROR_H
#define ERROR_H

#include "../scene.h"

typedef struct {
	char *msg;
} Error_Params;

extern Scene sceneError;

#endif
