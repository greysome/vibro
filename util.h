/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef UTIL
#define UTIL

#include <math.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define abs(x) ((x) < 0 ? -(x) : (x))
#define mod(a, b) (((a) % (b) < 0 ? (a) % (b) + (b) : (a) % (b)))
// C's fmodf behaves more like a remainder
#define fmodf2(a, b) \
  (fmodf((a), (b)) < 0 ? fmodf((a), (b)) + (b) : fmodf((a), (b)))

#endif