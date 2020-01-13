#include "bullets.h"

void generateBullet(struct bullet* bullets, size_t length, float angle, pointf pos, pointf playerSpeed)
{
    float speedMagnitude = 5.0;
    float playerSpeedInheritanceAmount = 1;

    struct bullet* b = (struct bullet*) searchForFreeSpace(bullets, sizeof(struct bullet), length);

    b->isActive = 1;
    b->position = pos;
    //b->speed = scalarMult(toVec(angle), speedMagnitude);

    b->speed = addVec(scalarMult(toVec(angle), speedMagnitude), scalarMult(playerSpeed, playerSpeedInheritanceAmount));

    //b->speed.x = -speedMagnitude * sin(angle) + playerSpeed.x * playerSpeedInheritanceAmount;
    //b->speed.y = speedMagnitude * cos(angle) + playerSpeed.y * playerSpeedInheritanceAmount;
}