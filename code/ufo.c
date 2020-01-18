#include "ufo.h"
#include "includes.h"
#include "bullets.h"
#include "tools.h"

void spawnUfo(struct ufo *ufo, uint8_t isSmall)
{
    ufo->isActive = 1;
    ufo->position = (pointf) {
        randRange(0, DISPLAY_SIZE_X), 
        randRange(0, DISPLAY_SIZE_Y)
    };

    changeUfoSpeed(ufo, 1);
    ufo->size = isSmall ? 3 : 5;
    ufo->health = 1;
    ufo->colliderRadius = 35;
}

void updateUfo(struct ufo *ufo)
{
    // I true with a probability of 1/100, which means it triggers
    // ~every 2s
    if (randRange(0, 1000) < 10)
    {
        changeUfoSpeed(ufo, 1);
    }

    addToVec(&ufo->position, ufo->speed);
    wrapScreen(&ufo->position);
}

void changeUfoSpeed(struct ufo *ufo, float maxSpeed)
{
    ufo->speed = randVect(-maxSpeed, maxSpeed);
}

uint8_t ufoShouldShoot(struct ufo *ufo)
{
    // 1/50 chance
    return randRange(0, 1000) < 25;
}

void ufoShoot(struct ufo *ufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength)
{
    pointf positionDifference = subVec(myplayer->position, ufo->position);

    float bulletSpeed = 3.0;
    // Player movement prediction
    // Not working correctly sometimes, TODO!
    float alpha = short_angle_dist(toAngle(positionDifference), toAngle(myplayer->speed));
    float targetAngle = toAngle(positionDifference) + (alpha * mag(myplayer->speed) / bulletSpeed);
    //float shootJitterDegrees = 5;
    //float shootJitterRad = shootJitterDegrees / 180 * M_PI;

    //float shootAngle = randRangef(targetAngle - shootJitterRad , targetAngle + shootJitterRad);

    //pointf shootSpeed = scalarMult(toVec(shootAngle), 2.0);

    generateBullet(bullets, bulletLength, targetAngle, bulletSpeed, 0.0, ufo->position, ufo->speed, FROM_UFO);
}

void drawUfo(struct ufo *ufo, color_t color)
{
    pointf ufoPosition;
    int scale = ufo->size;

    // Centering
    ufoPosition.x = ufo->position.x - 6.5*scale;
    ufoPosition.y = ufo->position.y - 3*scale;

    point ufoPoints[] = {{5 * scale, 0 * scale}, {8 * scale, 0 * scale}, {9 * scale, 2 * scale}, {13 * scale, 4 * scale}, {8 * scale, 6 * scale}, {5 * scale, 6 * scale}, {0 * scale, 4 * scale}, {4 * scale, 2 * scale}};
    gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, White);
    gdispDrawLine((4 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, (9 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, color);
    gdispDrawLine((0 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, (13 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, color);
}