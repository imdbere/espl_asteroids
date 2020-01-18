#include "includes.h"
#pragma once

struct player
{
    uint8_t isActive;
    pointf position;
    pointf speed;
    float angleRad;
    uint8_t isThrusting;
    uint8_t flameLength;
    float colliderRadius;
    int health;
	int score;
    char name[10];
};

void updatePlayer(struct player* player, int joyX, int joyY);
void drawPlayer(struct player* player);