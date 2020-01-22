#include "includes.h"
#include "explosion.h"

void drawExplosion(struct explosion *explosion)
{
    point position;
    if (explosion->frames < 200)
    {
        position.x = explosion->position.x;
        position.y = explosion->position.y + 5;
        point explosionPoints[] = {{0, 0}, {1, 3}, {3, 1}, {2, 4}, {5, 5}, {3, 6}, {4, 8}, {0, 6}, {-1, 9}, {-2, 6}, {-5, 7}, {-3, 4}, {-4, 2}, {-1, 3}};
        int pointCount = (int)sizeof(explosionPoints) / sizeof(point);
        for (int i = 0; i < pointCount; i++)
        {
            explosionPoints[i].x *= explosion->size;
            explosionPoints[i].y *= explosion->size;
        }
        if (explosion->fillPoly)
        {
            gdispFillConvexPoly(position.x, position.y, explosionPoints, pointCount, Orange);
        }
        else
        {
            gdispDrawPoly(position.x, position.y, explosionPoints, pointCount, Orange);
        }
        explosion->frames++;
    }
    else
    {
        position.x = explosion->position.x;
        position.y = explosion->position.y + 5;
        point explosionPoints[] = {{0, 0}, {2, 0}, {3, 1}, {1, 2}};

        pointf vertex;
        float currentAngle = 0;

        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                // explosionPoints[i].x = (explosionPoints[i].x + randRange(-3,3)) * explosion->size;
                // explosionPoints[i].y = (explosionPoints[i].y + randRange(-3,3)) * explosion->size;
            }
            vertex = scalarMult(toVec(currentAngle), (6 * explosion->size));
            gdispDrawPoly((int)vertex.x + position.x, (int)vertex.y + position.y, explosionPoints, 4, Orange);
            currentAngle += (2 * M_PI) / 7;
        }
    }
}