#include "states/main_menu.h"
#include "states/states.h"
#include "includes.h"
#include "input.h"
#include "asteroids.h"
#include "sm.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

TaskHandle_t mainMenuTaskHandle;
static gdispImage myImage;
static gdispImage titleImage;

int i, j;
char str[100];
int framesPerSecond = 0;

void mainMenuInit()
{ 
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
    GDisplay* g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(mainMenuTaskHandle);
    //xSemaphoreGive(g->mutex);
}

void mainMenuDrawTask(void *data)
{

    int asteroidCount = 10;
    struct asteroid asteroids[asteroidCount];
    generateAsteroids(&asteroids, asteroidCount, 20);
    struct buttons buttons;
    int selectorPositionY = 60;
    int selectedOffsetX[4] = {30, 0, 0, 0};
    int selectedBool = 0;
    int selected = 0;
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

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.E.risingEdge)
                {
                    if (selected == 0)
                    {
                        xQueueSend(state_queue, &gameStateId, 0);
                    }

                }
            }

            if(buttons.joystick.y < 0 && selectorPositionY > 60)
            {
                if(selectedBool == 0)
                {
                    selectorPositionY -= 30;
                    selected --;
                    selectedOffsetX[selected] = 30;
                    selectedOffsetX[selected + 1] = 0;
                    selectedBool = 1;
                }      
            }
            else if(buttons.joystick.y > 0 && selectorPositionY < 150)
            {
                if(selectedBool == 0)
                {
                    selectorPositionY += 30;
                    selected ++;
                    selectedOffsetX[selected] = 30;
                    selectedOffsetX[selected - 1] = 0;
                    selectedBool = 1;
                } 
            }
            else
            {
                selectedBool = 0;
            }            

            point points[] = {{0,0}, {0,16}, {20,8}};

            gdispFillConvexPoly(50, selectorPositionY, points, 3, White);

            sprintf(str, "Asteroids %i", buttons.joystick.y);
            gdispDrawString(50, 10, str, font32, White);

            sprintf(str, "Start Game");
            gdispDrawString(50+selectedOffsetX[0], 60, str, font1, White);
            
            sprintf(str, "Mode:");
            gdispDrawString(50+selectedOffsetX[1], 90, str, font1, White);
            int offset = gdispGetStringWidth(str,font1);

            sprintf(str, "  Single Player");
            gdispDrawString(50+offset+selectedOffsetX[1], 90, str, font1, White);

            sprintf(str, "High Score");
            gdispDrawString(50+selectedOffsetX[2], 120, str, font1, White);

            sprintf(str, "Name");
            gdispDrawString(50+selectedOffsetX[3], 150, str, font1, White);
             
            sprintf(str, "Frames %i", framesPerSecond++);
            gdispDrawString(DISPLAY_SIZE_X-100, DISPLAY_SIZE_Y-20, str, font1, White);

            // sprintf(str, "", swi)
        }
    }
}