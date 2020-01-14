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

void generateAsteroids(struct asteroid *asteroids, int maxAsteroidCount, int asteroidsCount, pointf pos, int radius);
void destroyAsteroid(struct asteroid asteroids[], int numAsteroids, int index);
void updateAsteroids(struct asteroid *asteroids, int asteroidCount);
void drawAsteroids(struct asteroid *asteroids, int asteroidCount, color_t color);

