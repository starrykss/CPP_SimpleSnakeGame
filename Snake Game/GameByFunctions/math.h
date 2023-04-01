#ifndef MATH_H
#define MATH_H

#include <SDL.h>

typedef struct v2
{
	int x, y;
} v2;

v2 createV2(int x, int y);
v2 v2Add(v2 value1, v2 value2);
v2 v2Mul(v2 vector, int scalar);
v2 v2Lerp(v2 start, v2 end, float amount);
SDL_bool v2Equals(v2 value1, v2 value2);

#endif 