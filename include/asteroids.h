#include "includes.h"

struct asteroid 
{
    point position;
    float speedX;
    float speedY;
    
    int segmentCount;
    point vertices[10];
};

void generateAsteroids(struct asteroid* asteroids, int asteroidsCount, int radius);
void drawAsteroids(struct asteroid* asteroids, int asteroidCount);

