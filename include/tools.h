#include "includes.h"
#include "math.h"

//uint32_t rand(void);

pointf addVec(pointf p1, pointf p2);
pointf subVec(pointf p1, pointf p2);
point toPoint(pointf p);
pointf toPointf(point p);
void addToVec(pointf *target, pointf toAdd);
void subFromVec(pointf *target, pointf toSub);
uint8_t cirlceTouchingCircle(pointf pos1, float rad1, pointf pos2, float rad2);
float floatMod(float a, float b);
float lerp_angle(float from, float to, float weight);
uint8_t pointWithinCircle(pointf circlePos, float circleRadius, pointf point);
int randRange(int lower, int upper);
float randRangef(float lower, float upper);
pointf rotatePoint(pointf p, float angle);
pointf scalarMult(pointf vec, float scal);
float short_angle_dist(float from, float to);
float square(float nr);
float toAngle(pointf vec);
pointf toVec(float angle);
pointf randVect(float lower, float upper);
float mag(pointf vec);
float normalizeAngle(float angle);
void clamp(float* val, float max);
void clampVec(pointf* vec, float max);
void wrapScreen(pointf* pos);
uint8_t isOutsideScreen(pointf p);
void* searchForFreeSpace(void *buffer, size_t structLength, size_t arrayLength);
uint8_t isEmpty(void *buffer, size_t structLength, size_t arrayLength);
void inactivateArray(void *buffer, size_t structLength, size_t arrayLength);

color_t HSVtoRGB(int H, float S, float V);