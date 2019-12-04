#include "states/main_menu.h"
#include "includes.h"

static gdispImage myImage;
static gdispImage titleImage;
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

void mainMenuDrawTask(void *data)
{
    coord_t swidth, sheight;
    swidth = gdispGetWidth();
    sheight = gdispGetHeight();
    int framesPerSecond=0;

    sprintf(str, "Asteroids %i", swidth);

    //gdispImageOpenFile(&myImage, "sprites.png");
    // gdispImageClose(&myImage);

    gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
    // gdispImageClose(&myImage);
    

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);

            //gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
            gdispImageDraw(&titleImage, 30, 30, 210, 40, 0, 0);
            
            

            /* sprintf(str, "Asteroids");
            gdispDrawString(10, 10, str, font32, White);

            sprintf(str, "Start Game");
            gdispDrawString(10, 50, str, font32, White);

            sprintf(str, "Mode:");
            gdispDrawString(10, 90, str, font1, White);

            sprintf(str, "Single Player");
            gdispDrawString(100, 130, str, font32, White); */

            sprintf(str, "Options %i", framesPerSecond++);
            gdispDrawString(10, 170, str, font32, White);

            // sprintf(str, "", swi)
        }
    }
}