#include "states/main_menu.h"
#include "includes.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

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

    int points = 6;
    int radius = 30;
    int offset[2] = {40, 40};

    int framesPerSecond = 0;
    float asteroidPoints[10][2][points];
    int upper = radius/2;
    int lower = radius/2*(-1);

    float current = 0;
    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < points; i++)
        {
            asteroidPoints[j][0][i] = sin(current) * radius;
            asteroidPoints[j][1][i] = cos(current) * radius;
            asteroidPoints[j][0][i] += (rand() % (upper - lower + 1)) + lower;
            asteroidPoints[j][1][i] += (rand() % (upper - lower + 1)) + lower;
            current += (2 * PI) / points;
        }
    }

    //sprintf(str, "Asteroids %i", swidth);

    //gdispImageOpenFile(&myImage, "sprites.png");
    // gdispImageClose(&myImage);

    gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
    // gdispImageClose(&myImage);
    j = 0;
    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);

            //gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
            
            
            for (j = 0; j < 10; j++)
            {
                for (i = 0; i < points - 1; i++)
                {
                    gdispDrawLine(asteroidPoints[j][0][i] + offset[0], asteroidPoints[j][1][i] + offset[1], asteroidPoints[j][0][i + 1] + offset[0], asteroidPoints[j][1][i + 1] + offset[1], White);
                }
                gdispDrawLine(asteroidPoints[j][0][i] + offset[0], asteroidPoints[j][1][i] + offset[1], asteroidPoints[j][0][0] + offset[0], asteroidPoints[j][1][0] + offset[1], White);
                offset[0] += 50;
                offset[1] += 50;
            }

            //gdispImageDraw(&titleImage, 30, 30, 210, 40, 0, 0);
            /* sprintf(str, "Asteroids");
            gdispDrawString(10, 10, str, font32, White);

            sprintf(str, "Start Game");
            gdispDrawString(10, 50, str, font32, White);

            sprintf(str, "Mode:");
            gdispDrawString(10, 90, str, font1, White);*/
            

            sprintf(str, "Frames %i", framesPerSecond++);
            gdispDrawString(10, 170, str, font32, White);

            // sprintf(str, "", swi)
        }
    }
}