#include "asteroids.h"
#include "includes.h"

void generateAsteroids(struct asteroid *asteroids, int maxAsteroidCount, int asteroidsCount, pointf pos, int radius)
{
    int upper = radius / 2;
    int lower = radius / 2 * (-1);

    float current = 0;
    float positionRandomness = 5;

    for (int j = 0; j < asteroidsCount; j++)
    {
        struct asteroid* newAsteroid = (struct asteroid*) searchForFreeSpace(asteroids, sizeof(struct asteroid), maxAsteroidCount);

        // Free space available
        if (newAsteroid != NULL)
        {
            newAsteroid->isActive = 1;

            int segmentCount = randRange(5, 10); //attention
            newAsteroid->segmentCount = segmentCount;
            newAsteroid->position = (pointf) {
                randRange(pos.x - positionRandomness, pos.x + positionRandomness), 
                randRange(pos.y - positionRandomness, pos.y + positionRandomness)};
            newAsteroid->speed.x = (randRange(-50, 50)) / 50.0;
            newAsteroid->speed.y = (randRange(-50, 50)) / 50.0;
            newAsteroid->radius = radius;

            for (int i = 0; i < segmentCount; i++)
            {
                point vertex;

                vertex.x = sin(current) * radius;
                vertex.y = cos(current) * radius;
                //Add Strange numbers
                vertex.x += randRange(lower, upper);
                vertex.y += randRange(lower, upper);

                newAsteroid->vertices[i] = vertex;
                current += (2 * M_PI) / segmentCount;
            }
        }


    }
}

void drawAsteroids(struct asteroid *asteroids, int asteroidCount, color_t color)
{
    for (int j = 0; j < asteroidCount; j++)
    {
        struct asteroid asteroid = asteroids[j];
        if (!asteroid.isActive) continue;

        for (int i = 0; i < asteroid.segmentCount; i++)
        {
            int nextVertex = i + 1;
            if (nextVertex == asteroid.segmentCount)
                nextVertex = 0;
            gdispDrawLine(
                asteroid.vertices[i].x + asteroid.position.x,
                asteroid.vertices[i].y + asteroid.position.y,
                asteroid.vertices[nextVertex].x + asteroid.position.x,
                asteroid.vertices[nextVertex].y + asteroid.position.y,
                color);
        }
        if (asteroids[j].position.x > DISPLAY_SIZE_X)
            asteroids[j].position.x = 0;
        if (asteroids[j].position.y > DISPLAY_SIZE_Y)
            asteroids[j].position.y = 0;
        if (asteroids[j].position.x < 0)
            asteroids[j].position.x = DISPLAY_SIZE_X;
        if (asteroids[j].position.y < 0)
            asteroids[j].position.y = DISPLAY_SIZE_Y;

        asteroids[j].position.x += asteroid.speed.x;
        asteroids[j].position.y += asteroid.speed.y;
    }
}