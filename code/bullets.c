#include "bullets.h"

void generateBullet(struct bullet* bullets, size_t length, float angle, float speed, float speedInheritance, pointf pos, pointf playerSpeed, enum BulletType type)
{
    struct bullet* b = (struct bullet*) searchForFreeSpace(bullets, sizeof(struct bullet), length);

    b->isActive = 1;
    b->position = pos;
    //b->speed = scalarMult(toVec(angle), speedMagnitude);

    b->speed = addVec(scalarMult(toVec(angle), speed), scalarMult(playerSpeed, speedInheritance));
    b->type = type;
    //b->speed.x = -speedMagnitude * sin(angle) + playerSpeed.x * playerSpeedInheritanceAmount;
    //b->speed.y = speedMagnitude * cos(angle) + playerSpeed.y * playerSpeedInheritanceAmount;
}

void drawBullets(struct bullet* bullets, int numBullets)
{
    for (int i=0; i<numBullets; i++)
    {
        struct bullet* b = &bullets[i];
        if (b->isActive)
        {
            color_t color = b->type == FROM_UFO ? Red : Blue;
            gdispFillCircle(b->position.x, b->position.y, 3, color);

            b->position.x += b->speed.x;
            b->position.y += b->speed.y;

            if (b->position.x > DISPLAY_SIZE_X ||
                b->position.y > DISPLAY_SIZE_Y ||
                b->position.x < 0 ||
                b->position.y < 0)
            {
                b->isActive = 0;
            }
        }
    }
}