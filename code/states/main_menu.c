#include "states/main_menu.h"
#include "includes.h"

font_t font;
char str[100] = "Hello";

void mainMenuInit()
{
    font = gdispOpenFont("DejaVuSans12");
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
    sprintf(str, "Asteroids %i", swidth);

    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(White);
        //gdispClear(White);
        
        gdispDrawString(30, 20, str, font, Black);
		gdispDrawBox(swidth - 120,  60, 100, 50, Black);

        
    }
}