/*
 * Some math macros.
 */

#ifndef MACROS_H
#define MACROS_H

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define clamp(x, a, b) (min((b), max((x), (a))))

#endif
