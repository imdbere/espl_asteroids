#include "includes.h"


struct ufo
{
    uint8_t isActive;
    uint8_t multiPlayerMode;
    pointf position;
    pointf speed;
    int size;
};

void changeUfoSpeed(struct ufo *myufo);
void drawUfo(struct ufo *myufo, color_t color);
void spawnUfo(struct ufo *myufo, uint8_t isSmall);
void ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength);
void updateUfo(struct ufo *myufo);