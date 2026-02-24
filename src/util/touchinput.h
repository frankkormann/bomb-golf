/*
 * Wrapper around HID for nicer detection of touchscreen inputs.
 */

#ifndef TOUCHINPUT_H
#define TOUCHINPUT_H

#include <3ds.h>

typedef struct {
	touchPosition start;
	touchPosition end;
	unsigned int length;
} TouchInput_Swipe;

/*
 * Call this once per frame after hidScanInput.
 */
void TouchInput_Scan();

/*
 * Returns true if the player started touching the touchscreen this frame.
 */
bool TouchInput_JustStarted();

/*
 * Returns true if the player is currently touching the touchscreen.
 *
 * This will return true when TouchInput_JustStarted returns true and false
 * when TouchInput_JustFinished returns true.
 */
bool TouchInput_InProgress();

/*
 * Returns true if the player was touching the touchscreen last frame but isn't
 * anymore.
 */
bool TouchInput_JustFinished();

/*
 * Returns the current swipe.
 *
 * If TouchInput_InProgress returns true, TouchInput_Swipe.end will be where
 * the player is currently touching.
 *
 * The return value has no meaning if TouchInput_InProgress and
 * TouchInput_JustFinished both return false.
 */
TouchInput_Swipe TouchInput_GetSwipe();

#endif
