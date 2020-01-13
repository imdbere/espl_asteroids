#include "tools.h"

int rotatePointX (int x, int y, float angle) 
{
    return (int) (cos(angle)*(x ) - sin(angle)*(y )) ;
}

int rotatePointY (int x, int y, float angle) 
{
    return -(int) (sin(angle)*(x ) + cos(angle)*(y )) ;
}

/*float lerp_angle(float a, float b, float t)
{https://www.google.com/
    if (abs(a-b) >= M_PI)
        if (a > b):
            a = normalize_angle(a) - 2.0 * M_PI
        else
            b = normalize_angle(b) - 2.0 * M_PI
    return lerp(a, b, t)
}*/
float floatMod(float a, float b)
{
    return (a - b * floor(a / b));
}

float short_angle_dist(float from, float to)
{
    float max_angle = M_PI * 2;
    float difference = floatMod(to - from, max_angle);
    return floatMod(2 * difference, max_angle) - difference;
}

float lerp_angle(float from, float to, float weight)
{
    return from + short_angle_dist(from, to) * weight;
}

pointf addVec(pointf p1, pointf p2)
{
    pointf p = {p1.x + p2.x, p1.y + p2.y};
    return p;
}

float square(float nr)
{
    return nr * nr;
}

uint8_t cirlceTouchingCircle(pointf pos1, float rad1, pointf pos2, float rad2)
{
    return square(pos1.x - pos2.x) + square(pos1.y - pos2.y) < square(rad1 + rad2);
}

uint8_t pointWithinCircle(pointf circlePos, float circleRadius, pointf point)
{
    return cirlceTouchingCircle(circlePos, circleRadius, point, 0);
}

int randRange(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

int randRangef(float lower, float upper)
{
    return (rand() / 65536.0 * (upper - lower)) + lower;
}

float toAngle(pointf vec)
{
	return atan2f(-vec.x, vec.y);
}

pointf toVec(float angle)
{
	return (pointf) {cos(angle), -sin(angle)};
}

pointf scalarMult(pointf vec, float scal)
{
	return (pointf) {vec.x * scal, vec.y * scal};
}

void* searchForFreeSpace(void *buffer, size_t structLength, size_t arrayLength)
{
    int i =0;
    uint8_t* data = (uint8_t*) buffer;

    for (i=0; i<arrayLength; i+=structLength)
    {
        if (!data[i])
           break;
    }

    if (i < arrayLength)
    {
        return (void*) buffer + i;
    }

    return NULL;
}