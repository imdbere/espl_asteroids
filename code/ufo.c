#include "ufo.h"
#include "includes.h"


void initUfo(struct ufo *myufo)
{
    
    
    
}
void drawUfo(struct ufo *myufo)
{
    int scale = myufo->size;
    point ufoPoints[] = {{5*scale,0*scale}, {8*scale,0*scale}, 
                            {9*scale,2*scale}, {13*scale,4*scale}, 
                                {8*scale,6*scale}, {5*scale,6*scale}, 
                                    {0*scale,4*scale}, {4*scale,2*scale}};
    gdispDrawPoly(myufo->position.x, myufo->position.y, ufoPoints, 8, White);
    gdispDrawLine((4*scale)+myufo->position.x,(2*scale)+myufo->position.y, (9*scale)+myufo->position.x,(2*scale)+myufo->position.y, White);
    gdispDrawLine((0*scale)+myufo->position.x,(4*scale)+myufo->position.y, (13*scale)+myufo->position.x,(4*scale)+myufo->position.y, White);
    // gdispFillCircle(myufo->position.x, myufo->position.y, 40, White);
    // gdispFillEllipse(myufo->position.x, myufo->position.y+10, 60, 20, White);
    
    
}