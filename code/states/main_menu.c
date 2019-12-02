#include "states/main_menu.h"
#include "includes.h"

static gdispImage myImage;
font_t font;
char str[100];
TaskHandle_t mainMenuTaskHandle;

void mainMenuInit()
{
    font = gdispOpenFont("DejaVuSansBold12_aa");
    sprintf(str, "Hello");
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 200, NULL, 3, &mainMenuTaskHandle);
    vTaskSuspend(mainMenuTaskHandle);
}

void mainMenuEnter() 
{
    vTaskResume(mainMenuTaskHandle);   
}

void mainMenuExit()
{
    vTaskSuspend(mainMenuTaskHandle);
}


void mainMenuDrawTask(void* data)
{
    coord_t	swidth, sheight;
    swidth = gdispGetWidth();
	sheight = gdispGetHeight();

    sprintf(str, "Asteroids %i", swidth);

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            gdispDrawBox(100, 100, 100, 100, White);
            //gdispClear(White);
            
            gdispDrawString(30, 20, str, font1, White);
            gdispDrawBox(swidth - 120,  60, 100, 50, White);

            
        }
    }

}