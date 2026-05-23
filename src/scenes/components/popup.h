/*
 * Modal dialog box which should be drawn on the bottom screen. Calling code is
 * responsible for pausing updates to its own state until the popup is dismissed.
 */

#ifndef POPUP_H
#define POPUP_H

#include <stdbool.h>

typedef enum {
	/*
	 * Popup message here
	 * [     Button     ]
	 */
	ONE_BUTTON,
	/*
	 * Popup message here
	 * [ Btn1  ][ Btn2  ]
	 */
	TWO_BUTTON
} Popup_Format;

typedef struct {
	char *label;
	void *onTouchParam;
	void (*onTouch)(void *param);
} Popup_Button;

/*
 * Creates and displays the popup. The size of buttons must match the option
 * selected for format.
 *
 * Only one popup can be displayed at a time. If there is already a popup
 * displayed, returns false.
 *
 * Returns false if there isn't enough memory.
 */
bool Popup_Init(char *message, Popup_Format format, Popup_Button buttons[]);

/*
 * Makes the popup go away. This function is best used in a Popup_Button
 * callback.
 */
void Popup_Exit();

/*
 * Call this once every frame that the popup is displayed.
 */
void Popup_Update();

/*
 * Draws the popup to the top layer and draws a translucent gray layer over
 * everything else.
 *
 * Call this after drawing everything else to ensure the layers are drawn
 * correctly.
 */
void Popup_Draw();

#endif
