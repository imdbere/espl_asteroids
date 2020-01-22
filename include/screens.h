#pragma once

#include "includes.h"
#include "states/states.h"
#include "states/level_change_screen.h"
#include "sm.h"
#include "player.h"
#include "states/game.h"

struct userScore
{
    uint8_t gameMode;
    char name[10];
    int score;
};

void looseGame(struct player *player, uint8_t gameMode);
void nextLevel(struct player *player, uint8_t gameMode);
void pauseGame(struct player *player, uint8_t gameMode);
void showDisconnected(struct player *player);
void winGame(struct player *player, uint8_t gameMode);