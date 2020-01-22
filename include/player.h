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
    int scoreOld;
    char name[10];
    int level;
    TickType_t isImmune;
    long immunityStartTime;
};

void updatePlayer(struct player* player, int joyX, int joyY);
uint8_t damagePlayer(struct player *player, uint8_t gameMode);
void drawPlayer(struct player* player);