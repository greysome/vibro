/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef UTIL
#define UTIL

#include <math.h>

#define dist(x1,y1,x2,y2) (((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define square(x) ((x)*(x))
#define clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define abs(x) ((x) < 0 ? -(x) : (x))
#define mod(a, b) (((a) % (b) < 0 ? (a) % (b) + (b) : (a) % (b)))
// C's fmodf behaves more like a remainder
#define fmodf2(a, b) \
  (fmodf((a), (b)) < 0 ? fmodf((a), (b)) + (b) : fmodf((a), (b)))

#endif