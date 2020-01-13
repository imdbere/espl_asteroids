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

    changeUfoSpeed(myufo, 1);
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

void changeUfoSpeed(struct ufo *myufo, float maxSpeed)
{
    myufo->speed = (pointf) {
        randRange(-maxSpeed, maxSpeed), 
        randRange(-maxSpeed, maxSpeed)
    };
}

uint8_t ufoShouldShoot(struct ufo *myufo)
{
    //TickType_t currentTicks = xTaskGetTickCount();
    // 1/50 chance
    if (randRange(0, 1000) < 25)
    {
        return TRUE;
    }
    return FALSE;
}

void __attribute__((optimize("O0")))  ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength)
{
    pointf positionDifference = (pointf) {
        myplayer->position.x - myufo->position.x,
        myplayer->position.y - myufo->position.y,
    };

    float bulletSpeed = 3.0;
    // Player movement prediction
    // Not working correctly, TODO!
    float alpha = short_angle_dist(toAngle(positionDifference), toAngle(myplayer->speed));
    float targetAngle = toAngle(positionDifference) + (alpha * mag(myplayer->speed) / bulletSpeed);
    //float shootJitterDegrees = 5;
    //float shootJitterRad = shootJitterDegrees / 180 * M_PI;

    //float shootAngle = randRangef(targetAngle - shootJitterRad , targetAngle + shootJitterRad);

    //pointf shootSpeed = scalarMult(toVec(shootAngle), 2.0);

    generateBullet(bullets, bulletLength, targetAngle, bulletSpeed, 0.0, myufo->position, myufo->speed, FROM_UFO);
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