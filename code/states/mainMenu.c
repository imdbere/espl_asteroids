#include "states/mainMenu.h"
#include "includes.h"

static gdispImage myImage;
static gdispImage titleImage;
font_t font;
char str[100];

void mainMenuInit()
{
    font = gdispOpenFont("UI1");
    sprintf(str, "Error");
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

    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(Black);
        
        gdispImageOpenFile(&myImage, "sprites.png");
        gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
        gdispImageClose(&myImage);
        
        gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
        gdispImageDraw(&titleImage, 40, 10, titleImage.width, titleImage.height, 0, 0);
        gdispImageClose(&myImage);
        
        sprintf(str, "Asteroids");
        gdispDrawString(10, 10, str, font32, White);

        sprintf(str, "Start Game");
        gdispDrawString(10, 50, str, font32, White);

        sprintf(str, "Mode:");
        gdispDrawString(10, 90, str, font1, White);

        sprintf(str, "Single Player");
        gdispDrawString(100, 130, str, font32, White);

        sprintf(str, "Options");
        gdispDrawString(10, 170, str, font32, White);

        // sprintf(str, "", swi)
          
    }
}