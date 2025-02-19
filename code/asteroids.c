#include "asteroids.h"
#include "includes.h"
#include "tools.h"

void generateAsteroids(struct asteroid *asteroids, int asteroidsLength, int asteroidsCount, pointf pos, int radius)
{
    int upper = radius / 2;
    int lower = radius / 2 * (-1);

    float positionRandomness = 5;

    for (int j = 0; j < asteroidsCount; j++)
    {
        struct asteroid* newAsteroid = (struct asteroid*) searchForFreeSpace(asteroids, sizeof(struct asteroid), asteroidsLength);

        // Free space available
        if (newAsteroid != NULL)
        {
            newAsteroid->isActive = 1;
            newAsteroid->position = addVec(pos, randVect(-positionRandomness, positionRandomness));

            // Smaller asteroids are faster
            float speedMagnitude = 15.0/radius;
            newAsteroid->speed = randVect(-speedMagnitude, speedMagnitude);
            newAsteroid->radius = radius;

            int segmentCount = randRange(5, 10);
            newAsteroid->segmentCount = segmentCount;

            float currentAngle = 0;
            for (int i = 0; i < segmentCount; i++)
            {
                pointf vertex = scalarMult(toVec(currentAngle), radius);
                // Randomize Asteroid
                addToVec(&vertex, randVect(lower, upper));

                newAsteroid->vertices[i] = toPoint(vertex);
                currentAngle += (2 * M_PI) / segmentCount;
            }
        }
    }
}

void destroyAsteroid(struct asteroid asteroids[], int numAsteroids, int index)
{
    struct asteroid *a = &asteroids[index];
    a->isActive = 0;
    int radius = a->radius - 10;

    if (radius > 0)
        generateAsteroids(asteroids, numAsteroids * sizeof(struct asteroid), 2, a->position, radius);
}

void updateAsteroids(struct asteroid *asteroids, int asteroidCount)
{
    for (int j = 0; j < asteroidCount; j++)
    {
        struct asteroid asteroid = asteroids[j];
        if (!asteroid.isActive) continue;

        addToVec(&asteroids[j].position, asteroid.speed);
        wrapScreen(&asteroids[j].position);
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
            // Close circle with last segment
            if (nextVertex == asteroid.segmentCount)
                nextVertex = 0;

            gdispDrawLine(
                asteroid.vertices[i].x + asteroid.position.x,
                asteroid.vertices[i].y + asteroid.position.y,
                asteroid.vertices[nextVertex].x + asteroid.position.x,
                asteroid.vertices[nextVertex].y + asteroid.position.y,
                color);
        }
        
    }
}