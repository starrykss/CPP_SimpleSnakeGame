#include "math.h"

v2 createV2(int x, int y)
{
	v2 result = { x, y };
	return result;
}

v2 v2Add(v2 value1, v2 value2)
{
	v2 result = { value1.x + value2.x, value1.y + value2.y };
	return result;
}

v2 v2Mul(v2 vector, int scalar)
{
	v2 result = { vector.x * scalar, vector.y * scalar };
	return result;
}

v2 v2Lerp(v2 start, v2 end, float amount)
{
	if (amount < 0)
	{
		amount = 0;
	}
	else if (amount > 1)
	{
		amount = 1;
	}

	v2 result;
	result.x = (int)((1 - amount) * start.x + amount * end.x);
	result.y = (int)((1 - amount) * start.y + amount * end.y);
	return result;
}

SDL_bool v2Equals(v2 value1, v2 value2)
{
    return (value1.x == value2.x) && (value1.y == value2.y);
}