#include "bullets.h"

void generateBullet(struct bullet* bullets, size_t length, float angle, float speed, float speedInheritance, pointf pos, pointf playerSpeed, enum BulletType type)
{
    struct bullet* b = (struct bullet*) searchForFreeSpace(bullets, sizeof(struct bullet), length);
    
    b->isActive = 1;
    b->position = pos;

    b->speed = addVec(scalarMult(toVec(angle), speed), scalarMult(playerSpeed, speedInheritance));
    b->type = type;
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

void drawBullets(struct bullet* bullets, int bulletsLength)
{
    for (int i=0; i<bulletsLength; i++)
    {
        struct bullet* b = &bullets[i];
        if (!b->isActive) continue;

        color_t color = b->type == FROM_UFO ? Red : Blue;
        gdispFillCircle(b->position.x, b->position.y, 3, color);
    }
}