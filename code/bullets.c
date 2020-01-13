#include "bullets.h"

void generateBullet(struct bullet* bullets, size_t length, pointf pos, pointf playerSpeed, float angle)
{
    float speedMagnitude = 5.0;
    float playerSpeedInheritanceAmount = 1;

    struct bullet* b = (struct bullet*) searchForFreeSpace(bullets, sizeof(struct bullet), length);
    
    b->isActive = 1;
    b->position = pos;
    b->speed.x = -speedMagnitude * sin(angle) + playerSpeed.x * playerSpeedInheritanceAmount;
    b->speed.y = speedMagnitude * cos(angle) + playerSpeed.y * playerSpeedInheritanceAmount;
}