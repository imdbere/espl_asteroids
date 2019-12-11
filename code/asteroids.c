#include "asteroids.h"
#include "includes.h"

int randRange(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

void generateAsteroids(struct asteroid *asteroids, int asteroidsCount, int radius)
{
    int upper = radius / 2;
    int lower = radius / 2 * (-1);

    float current = 0;
    for (int j = 0; j < asteroidsCount; j++)
    {
        int segmentCount = randRange(5, 10); //attention
        asteroids[j].segmentCount = segmentCount;
        asteroids[j].position = (pointf){randRange(0, 100), randRange(0, 100)};
        asteroids[j].speed.x = (randRange(-50, 50)) / 50.0;
        asteroids[j].speed.y = (randRange(-50, 50)) / 50.0;
        asteroids[j].radius = radius;

        for (int i = 0; i < segmentCount; i++)
        {
            point vertex;

            vertex.x = sin(current) * radius;
            vertex.y = cos(current) * radius;
            //Add Strange numbers
            vertex.x += randRange(lower, upper);
            vertex.y += randRange(lower, upper);

            asteroids[j].vertices[i] = vertex;
            current += (2 * M_PI) / segmentCount;
        }
    }
}

void drawAsteroids(struct asteroid *asteroids, int asteroidCount)
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
                White);
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