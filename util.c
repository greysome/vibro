int clamp(int x, int min, int max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

float fclamp(float x, float min, float max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}