#include "states/main_menu.h"
#include "includes.h"
#include "asteroids.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846
#define X 0
#define Y 1

TaskHandle_t mainMenuTaskHandle;
static gdispImage myImage;
static gdispImage titleImage;
font_t font;

int i, j;
char str[100];

void mainMenuInit()
{
    font = gdispOpenFont("DejaVuSansBold12_aa");
    sprintf(str, "Hello");
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 2000, NULL, 3, &mainMenuTaskHandle);
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

    int asteroidCount = 10;
    struct asteroid asteroids[asteroidCount];
    generateAsteroids(&asteroids, asteroidCount, 20);
    //sprintf(str, "Asteroids %i", swidth);

    //gdispImageOpenFile(&myImage, "sprites.png");
    // gdispImageClose(&myImage);

    gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
    // gdispImageClose(&myImage);
    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);

            drawAsteroids(&asteroids, asteroidCount);
            //gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
            //gdispImageDraw(&titleImage, 30, 30, 210, 40, 0, 0);
            /* sprintf(str, "Asteroids");
            gdispDrawString(10, 10, str, font32, White);

            sprintf(str, "Start Game");
            gdispDrawString(10, 50, str, font32, White);

            sprintf(str, "Mode:");
            gdispDrawString(10, 90, str, font1, White);*/
            

            //sprintf(str, "Frames %i", framesPerSecond++);
            //gdispDrawString(10, 170, str, font32, White);

            // sprintf(str, "", swi)
        }
    }
}