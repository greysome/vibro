/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef UTIL
#define UTIL

#include <math.h>

// A special null value meant for int variables
#define NIL -100

#define dist(x1,y1,x2,y2) (((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define square(x) ((x)*(x))
// Returns min if x < min, max if x > max, and x otherwise
#define clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define abs(x) ((x) < 0 ? -(x) : (x))
#define mod(a, b) (((a) % (b) < 0 ? (a) % (b) + (b) : (a) % (b)))
// C's fmodf does not truly behave like mod for a < 0, rather it behaves
// like a similar (but different) function called rem.
#define fmodf2(a, b) \
  (fmodf((a), (b)) < 0 ? fmodf((a), (b)) + (b) : fmodf((a), (b)))

#endif