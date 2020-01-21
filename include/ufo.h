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
    color_t color;
    //TickType_t lastShootTick;
};

void changeUfoSpeed(struct ufo *myufo, float maxSpeed);
void drawUfo(struct ufo *ufos, uint8_t maxUfoCount);
void spawnUfo(struct ufo *myufo, uint8_t isSmall);
uint8_t spawnUfoRandom(struct ufo *ufos, size_t length);
void ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength);
void updateUfo(struct ufo *myufo, uint8_t maxUfoCount);
uint8_t ufoShouldShoot();