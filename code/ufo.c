#include "ufo.h"
#include "includes.h"
#include "bullets.h"

void spawnUfo(struct ufo *myufo, uint8_t isSmall)
{
    myufo->isActive = 1;
    myufo->position = (pointf) {
        randRange(0, DISPLAY_SIZE_X), 
        randRange(0, DISPLAY_SIZE_Y)
    };

    changeUfoSpeed(myufo);
    myufo->size = isSmall ? 3 : 5;
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

void changeUfoSpeed(struct ufo *myufo)
{
    myufo->speed = (pointf) {
        randRange(-2, 2), 
        randRange(-2, 2)
    };
}

void ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength)
{
    pointf positionDifference = (pointf) {
        myplayer->position.x - myufo->position.x,
        myplayer->position.y - myufo->position.y,
    };

    float targetAngle = toAngle(positionDifference);
    float shootAngle = randRangef(targetAngle - 5, targetAngle + 5);

    //pointf shootSpeed = scalarMult(toVec(shootAngle), 2.0);

    generateBullet(bullets, bulletLength, shootAngle, myufo->position, myufo->speed);
}

void drawUfo(struct ufo *myufo, color_t color)
{
    pointf ufoPosition;

    int scale = myufo->size;

    ufoPosition.x = myufo->position.x - 6.5*scale;
    ufoPosition.y = myufo->position.y - 3*scale;

    point ufoPoints[] = {{5 * scale, 0 * scale}, {8 * scale, 0 * scale}, {9 * scale, 2 * scale}, {13 * scale, 4 * scale}, {8 * scale, 6 * scale}, {5 * scale, 6 * scale}, {0 * scale, 4 * scale}, {4 * scale, 2 * scale}};
    gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, White);
    gdispDrawLine((4 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, (9 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, color);
    gdispDrawLine((0 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, (13 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, color);
}