/*
 * Scene to display an error message.
 */

#ifndef ERROR_H
#define ERROR_H

typedef struct {
	char *msg;
} Error_Params;

#include "../scene.h"

extern Scene sceneError;

/*
 * Makes Scene_Params to display the null-terminated string msg.
 */
Scene_Params Error_MakeParams(const char *msg);

#endif
