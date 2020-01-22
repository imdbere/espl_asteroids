#include "ufo.h"
#include "includes.h"
#include "bullets.h"
#include "tools.h"

void spawnUfo(struct ufo *ufo, uint8_t isSmall)
{
    ufo->isActive = 1;
    ufo->isImmune = 0;
    ufo->controlledByPlayer = 0;
    ufo->showHealth = 1;
    ufo->position = (pointf){
        randRange(0, DISPLAY_SIZE_X),
        randRange(0, DISPLAY_SIZE_Y)};

    changeUfoSpeed(ufo, 1);
    ufo->size = isSmall ? 3 : 5;
    ufo->health = UFO_MAX_LIFES_SP;
    ufo->maxHealth = UFO_MAX_LIFES_SP;
    ufo->colliderRadius = 35;
    ufo->color = HSVtoRGB(randRange(0, 360), 1, 1);
}

uint8_t spawnUfoRandom(struct ufo *ufos, size_t length)
{
    struct ufo *u = (struct ufo *)searchForFreeSpace(ufos, sizeof(struct ufo), length);
    if (u != NULL)
    {
        if (randRange(0, 1000) < 5)
        {
            spawnUfo(u, 1);
            return 1;
        }
    }
    return 0;
}

void updateUfo(struct ufo *ufos, uint8_t maxUfoCount)
{
    // I true with a probability of 1/100, which means it triggers
    // ~every 2s
    for (int i = 0; i < maxUfoCount; i++)
    {
        struct ufo *ufo = &ufos[i];
        if (!ufo->isActive) continue;
        
        if (randRange(0, 1000) < 10)
        {
            changeUfoSpeed(&ufos[i], 1);
        }
        addToVec(&ufo->position, ufo->speed);
        wrapScreen(&ufo->position);
    }
}

void changeUfoSpeed(struct ufo *ufo, float maxSpeed)
{
    ufo->speed = randVect(-maxSpeed, maxSpeed);
}

uint8_t ufoShouldShoot()
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

void damageUfo(struct ufo *ufo)
{
    ufo->health -= 1;
    if (ufo->controlledByPlayer)
    {
        ufo->isImmune = 1;
        ufo->immunityStartTime = xTaskGetTickCount();
    }

    if (ufo->health <= 0)
    {
        ufo->isActive = 0;
    }
}

void drawUfo(struct ufo *ufos, uint8_t maxUfoCount)
{
    pointf ufoPosition;
    for (int i = 0; i < maxUfoCount; i++)
    {
        struct ufo *ufo = &ufos[i];
        if (!ufo->isActive) continue;

        int scale = ufo->size;

        // Centering
        ufoPosition.x = ufo->position.x - 6.5 * scale;
        ufoPosition.y = ufo->position.y - 3 * scale;

        color_t outlineColor = ufo->isImmune ? Grey : White;
        point ufoPoints[] = {{5 * scale, 0 * scale}, {8 * scale, 0 * scale}, {9 * scale, 2 * scale}, {13 * scale, 4 * scale}, {8 * scale, 6 * scale}, {5 * scale, 6 * scale}, {0 * scale, 4 * scale}, {4 * scale, 2 * scale}};
        gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, outlineColor);
        gdispDrawLine((4 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, (9 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, ufo->color);
        gdispDrawLine((0 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, (13 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, ufo->color);

        // if(ufo->showHealt)

        if (ufo->showHealth)
        {
            gdispDrawBox(ufoPosition.x, ufoPosition.y - (scale * 3), 13 * scale, 1 * scale, Grey);
            gdispFillArea(ufoPosition.x, ufoPosition.y - (scale * 3),
                            (int)((ufo->health / (float)ufo->maxHealth) * 13.0) * scale, 1 * scale, Red);
        }

    }
}