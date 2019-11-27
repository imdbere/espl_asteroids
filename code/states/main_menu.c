#include "states/main_menu.h"
#include "includes.h"

font_t font;
char str[100];

void mainMenuInit()
{
    font = gdispOpenFont("DejaVuSansBold12_aa");
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

    sprintf(str, "Asteroids %i", swidth);

    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(Black);
        //gdispClear(White);
        
        gdispDrawString(30, 20, str, font1, White);
		gdispDrawBox(swidth - 120,  60, 100, 50, White);

        
    }
}