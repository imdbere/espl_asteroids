#include "includes.h"
#pragma once

enum BulletType 
{
    FROM_PLAYER,
    FROM_UFO
};

struct bullet 
{
    uint8_t isActive;
    pointf position;
    pointf speed;
    enum BulletType type;
};

void generateBullet(struct bullet* bullets, size_t length, float angle, float speed, float speedInheritance, pointf pos, pointf playerSpeed, enum BulletType type);
void drawBullets(struct bullet* bullets, int numBullets);