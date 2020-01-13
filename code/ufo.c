#include "ufo.h"
#include "includes.h"
#include "bullets.h"

void spawnUfo(struct ufo *myufo, uint8_t isSmall)
{
    myufo->isActive = 1;
    myufo->position = (pointf) {
        randRange(0, DISPLAY_SIZE_X), 
        randRange(0, DISPLAY_SIZE_Y)
    };

    changeUfoSpeed(myufo);
    myufo->size = isSmall ? 3 : 5;
}

void changeUfoSpeed(struct ufo *myufo)
{
    myufo->speed = (pointf) {
        randRange(-2, 2), 
        randRange(-2, 2)
    };
}

void ufoShoot(struct ufo *myufo, struct player *myplayer, struct bullet *bullets, size_t bulletLength)
{
    pointf positionDifference = (pointf) {
        myplayer->position.x - myufo->position.x,
        myplayer->position.y - myufo->position.y,
    };

    float targetAngle = toAngle(positionDifference);
    float shootAngle = randRangef(targetAngle - 5, targetAngle + 5);

    //pointf shootSpeed = scalarMult(toVec(shootAngle), 2.0);

    generateBullet(bullets, bulletLength, myufo->position, myufo->speed, shootAngle);
}

void drawUfo(struct ufo *myufo)
{
    pointf ufoPosition;
    
    int scale = myufo->size;

    ufoPosition.x = myufo->position.x - 6.5*scale;
    ufoPosition.y = myufo->position.y - 3*scale;

    point ufoPoints[] = {{5*scale,0*scale}, {8*scale,0*scale}, 
                            {9*scale,2*scale}, {13*scale,4*scale}, 
                                {8*scale,6*scale}, {5*scale,6*scale}, 
                                    {0*scale,4*scale}, {4*scale,2*scale}};
    gdispDrawPoly(ufoPosition.x, ufoPosition.y, ufoPoints, 8, White);
    gdispDrawLine((4*scale)+ufoPosition.x,(2*scale)+ufoPosition.y, (9*scale)+ufoPosition.x,(2*scale)+ufoPosition.y, White);
    gdispDrawLine((0*scale)+ufoPosition.x,(4*scale)+ufoPosition.y, (13*scale)+ufoPosition.x,(4*scale)+ufoPosition.y, White);
    // gdispFillCircle(myufo->position.x, myufo->position.y, 40, White);
    // gdispFillEllipse(myufo->position.x, myufo->position.y+10, 60, 20, White);
    
    
}