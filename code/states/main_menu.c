#include "states/main_menu.h"
#include "includes.h"

static gdispImage myImage;

void mainMenuInit()
{

}

void mainMenuEnter()
{

}

void mainMenuExit()
{

}


void mainMenuRun()
{
    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(Black);
		gdispDrawBox(100, 100, 100, 100, White);
        gdispImageOpenFile(&myImage, "sprites.png");
        gdispImageDraw(&myImage, 30, 30, 13, 18, 0, 14);
    }
}