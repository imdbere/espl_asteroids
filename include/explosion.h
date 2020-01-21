#pragma once

#include "includes.h"

struct explosion
{

    int size;
    uint8_t frames;
    pointf position;
    uint8_t fillPoly;
};

void drawExplosion(struct explosion *explosion);
