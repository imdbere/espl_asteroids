#include "includes.h"


struct ufo
{
    uint8_t isActive;
    uint8_t multiPlayerMode;
    pointf position;
    pointf speed;
    int size;
};

//void generateUfo(struct ufo* ufo);
void initUfo(struct ufo *ufo);
void updateUfo(struct ufo *myufo);
void drawUfo(struct ufo* ufo, color_t color);