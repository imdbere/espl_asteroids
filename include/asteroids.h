#include "includes.h"

struct asteroid 
{
    uint8_t isActive;
    pointf position;
    pointf speed;
    float radius;
    
    int segmentCount;
    point vertices[10];
};

void generateAsteroids(struct asteroid* asteroids, int asteroidsCount, int radius);
void drawAsteroids(struct asteroid* asteroids, int asteroidCount);

