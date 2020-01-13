#include "includes.h"
#include "math.h"

pointf addPoints(pointf p1, pointf p2);
uint8_t cirlceTouchingCircle(pointf pos1, float rad1, pointf pos2, float rad2);
float floatMod(float a, float b);
float lerp_angle(float from, float to, float weight);
uint8_t pointWithinCircle(pointf circlePos, float circleRadius, pointf point);
int randRange(int lower, int upper);
int rotatePointX(int x, int y, float angle);
int rotatePointY(int x, int y, float angle);
pointf scalarMult(pointf vec, float scal);
float short_angle_dist(float from, float to);
float square(float nr);
float toAngle(pointf vec);
pointf toVec(float angle);
void* searchForFreeSpace(void *buffer, size_t structLength, size_t arrayLength);