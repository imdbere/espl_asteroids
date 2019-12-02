#include "states/mainMenu.h"
#include "includes.h"

static gdispImage myImage;
font_t font;
char str[100];

void mainMenuInit()
{
    font = gdispOpenFont("UI1");
    sprintf(str, "Hello");
}

void mainMenuEnter() 
{
    
}

void mainMenuExit()
{

}


void mainMenuRun()
{
    coord_t	swidth, sheight;
    swidth = gdispGetWidth();
	sheight = gdispGetHeight();

    sprintf(str, "Asteroids", swidth);

    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(Black);
        
        gdispImageOpenFile(&myImage, "sprites.png");
        gdispImageDraw(&myImage, 30, 30, 28, 74, 59, 32);
        gdispImageClose(&myImage);
        
        gdispDrawString(10, 10, str, font32, White);
          
    }
}