#include "includes.h"
#include "bullets.h"
#include "player.h"

struct ufo
{
    uint8_t isActive;
    uint8_t collidesWithAsteroids;
    uint8_t showHealth;
    pointf position;
    pointf speed;
    uint8_t size;
    uint8_t health;
    uint8_t maxHealth;
    float colliderRadius;
    //TickType_t lastShootTick;
};

void changeUfoSpeed(struct ufo *myufo, float maxSpeed);
void drawUfo(struct ufo *myufo, color_t color);
void spawnUfo(struct ufo *myufo, uint8_t isSmall);
void ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength);
void updateUfo(struct ufo *myufo);
uint8_t ufoShouldShoot(struct ufo *myufo);