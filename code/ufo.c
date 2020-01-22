#include "ufo.h"
#include "includes.h"
#include "bullets.h"
#include "tools.h"

void spawnUfo(struct ufo *ufo, uint8_t isSmall)
{
    ufo->isActive = 1;
    ufo->collidesWithAsteroids = 0;
    ufo->position = (pointf){
        randRange(0, DISPLAY_SIZE_X),
        randRange(0, DISPLAY_SIZE_Y)};

    changeUfoSpeed(ufo, 1);
    ufo->size = isSmall ? 3 : 5;
    ufo->health = 1;
    ufo->colliderRadius = 35;
    ufo->color = HSVtoRGB(randRange(0, 360), 1, 1);
}

void spawnUfoRandom(struct ufo *ufos, size_t length)
{
    struct ufo *u = (struct ufo *)searchForFreeSpace(ufos, sizeof(struct ufo), length);
    if (u != NULL)
    {
        if (randRange(0, 1000) < 5)
        {
            spawnUfo(u, 1);
        }
    }
}

void updateUfo(struct ufo *ufos, uint8_t maxUfoCount)
{
    // I true with a probability of 1/100, which means it triggers
    // ~every 2s
    for (int i = 0; i < maxUfoCount; i++)
    {
        if (ufos[i].isActive)
        {
            if (randRange(0, 1000) < 10)
            {
                changeUfoSpeed(&ufos[i], 1);
            }
            addToVec(&ufos[i].position, ufos[i].speed);
            wrapScreen(&ufos[i].position);
        }
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
        if (ufos[i].isActive)
        {
            int scale = ufos[i].size;

            // Centering
            ufoPosition.x = ufos[i].position.x - 6.5 * scale;
            ufoPosition.y = ufos[i].position.y - 3 * scale;

            point ufoPoints[] = {{5 * scale, 0 * scale}, {8 * scale, 0 * scale}, {9 * scale, 2 * scale}, {13 * scale, 4 * scale}, {8 * scale, 6 * scale}, {5 * scale, 6 * scale}, {0 * scale, 4 * scale}, {4 * scale, 2 * scale}};
            gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, White);
            gdispDrawLine((4 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, (9 * scale) + ufoPosition.x, (2 * scale) + ufoPosition.y, ufos[i].color);
            gdispDrawLine((0 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, (13 * scale) + ufoPosition.x, (4 * scale) + ufoPosition.y, ufos[i].color);

            // if(ufo->showHealt)

            if (ufos[i].showHealth)
            {
                if (ufos[i].health > ufos[i].maxHealth)
                    ufos[i].health = ufos[i].maxHealth;
                if (ufos[i].health < 0)
                    ufos[i].health = 0;
                gdispDrawBox(ufoPosition.x, ufoPosition.y - (scale * 3), 13 * scale, 1 * scale, ufos[i].color);
                gdispFillArea(ufoPosition.x, ufoPosition.y - (scale * 3),
                              (int)((ufos[i].health / (float)ufos[i].maxHealth) * 13.0) * scale, 1 * scale, Red);
            }
        }
    }
}