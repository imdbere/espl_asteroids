#include "tools.h"


pointf rotatePoint(pointf p, float angle)
{
    return (pointf) {
        (cos(angle)*p.x - sin(angle)*p.y),
        -(sin(angle)*p.x + cos(angle)*p.y)
    };
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

point toPoint(pointf p)
{
    return (point) {(int)p.x, (int)p.y};
}

pointf toPointf(point p)
{
    return (pointf) {(float)p.x, (float)p.y};
}

pointf addVec(pointf p1, pointf p2)
{
    return (pointf) {p1.x + p2.x, p1.y + p2.y};
}

pointf subVec(pointf p1, pointf p2)
{
    return (pointf) {p1.x - p2.x, p1.y - p2.y};
}

void addToVec(pointf *target, pointf toAdd)
{
    target->x += toAdd.x;
    target->y += toAdd.y;
}

void subFromVec(pointf *target, pointf toSub)
{
    target->x -= toSub.x;
    target->y -= toSub.y;
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

float  __attribute__((optimize("O0"))) randRangef(float lower, float upper)
{
    int ranI = rand() % 65536;
    float ran = (float)ranI;
    return (ran / 65536.0 * (upper - lower)) + lower;
}

pointf randVect(float lower, float upper)
{
    return (pointf) {
        randRangef(lower, upper),
        randRangef(lower, upper)
    };
}

float toAngle(pointf vec)
{
	return atan2f(-vec.y, vec.x);
}

pointf toVec(float angle)
{
	return (pointf) {cos(angle), -sin(angle)};
}

pointf scalarMult(pointf vec, float scal)
{
	return (pointf) {vec.x * scal, vec.y * scal};
}

float mag(pointf vec)
{
    return sqrtf(square(vec.x) + square(vec.y));
}

float normalizeAngle(float angle)
{
    if (angle > M_PI)
        return 2*M_PI - angle;
    if (angle < -M_PI)
        return 2*M_PI + angle;
    
    return angle;
}

void clamp(float* val, float max)
{
    if (*val > max)
        *val = max;
    if (*val < -max)
        *val = -max;
}

void clampVec(pointf* vec, float max)
{
    clamp(&vec->x, max);
    clamp(&vec->y, max);
}

void wrapScreen(pointf* pos)
{
    if (pos->x > DISPLAY_SIZE_X)
        pos->x = 0;
    if (pos->y > DISPLAY_SIZE_Y)
        pos->y = 0;
    if (pos->x < 0)
        pos->x = DISPLAY_SIZE_X;
    if (pos->y < 0)
        pos->y = DISPLAY_SIZE_Y;
}

uint8_t isOutsideScreen(pointf p)
{
    return (p.x > DISPLAY_SIZE_X ||
        p.y > DISPLAY_SIZE_Y ||
        p.x < 0 ||
        p.y < 0);
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

void inactivateArray(void *buffer, size_t structLength, size_t arrayLength)
{
    int i =0;
    uint8_t* data = (uint8_t*) buffer;

    for (i=0; i<arrayLength; i+=structLength)
    {
        data[i] = 0;
    }
}