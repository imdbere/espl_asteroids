#include "ufo.h"
#include "includes.h"

void initUfo(struct ufo *myufo)
{
}

void updateUfo(struct ufo *myufo)
{
    myufo->position.x += myufo->speed.x;
    myufo->position.y += myufo->speed.y;
    if (myufo->position.x > DISPLAY_SIZE_X)
        myufo->position.x = 0;
    if (myufo->position.y > DISPLAY_SIZE_Y)
        myufo->position.y = 0;
    if (myufo->position.x < 0)
        myufo->position.x = DISPLAY_SIZE_X;
    if (myufo->position.y < 0)
        myufo->position.y = DISPLAY_SIZE_Y;
}

void drawUfo(struct ufo *myufo)
{
    pointf ufoPosition;

    int scale = myufo->size;

    ufoPosition.x = myufo->position.x - (13 / 2) * scale;
    ufoPosition.y = myufo->position.y - (6 / 2) * scale;

    point ufoPoints[] = {{5 * scale, 0 * scale}, {8 * scale, 0 * scale}, {9 * scale, 2 * scale}, {13 * scale, 4 * scale}, {8 * scale, 6 * scale}, {5 * scale, 6 * scale}, {0 * scale, 4 * scale}, {4 * scale, 2 * scale}};
    gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, White);
    gdispDrawLine((4 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, (9 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, White);
    gdispDrawLine((0 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, (13 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, White);
}