#include "bullets.h"

struct bullet* generateBullet(struct bullet* bullets, size_t length, float angle, float speed, float speedInheritance, pointf pos, pointf playerSpeed, enum BulletType type)
{
    struct bullet* b = (struct bullet*) searchForFreeSpace(bullets, sizeof(struct bullet), length);
    
    b->isActive = 1;
    b->position = pos;

    b->speed = addVec(scalarMult(toVec(angle), speed), scalarMult(playerSpeed, speedInheritance));
    b->type = type;

    return b;
}

void updateBullets(struct bullet* bullets, int bulletsLength)
{
    for (int i=0; i<bulletsLength; i++)
    {
        struct bullet* b = &bullets[i];
        if (!b->isActive) continue;

        addToVec(&b->position, b->speed);
        if (isOutsideScreen(b->position))
        {
            b->isActive = 0;
        }
    }
}

color_t getBulletColor(enum BulletType type)
{
    switch (type)
    {
        case FROM_PLAYER: return Blue;
        case FROM_PLAYER_UFO: return Green;
        case FROM_UFO: return Red;
    }
}

void drawBullets(struct bullet* bullets, int bulletsLength)
{
    for (int i=0; i<bulletsLength; i++)
    {
        struct bullet* b = &bullets[i];
        if (!b->isActive) continue;

        color_t color = getBulletColor(b->type);
        gdispFillCircle(b->position.x, b->position.y, 3, color);
    }
}