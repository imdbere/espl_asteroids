#include "states/main_menu.h"
#include "states/states.h"
#include "includes.h"
#include "input.h"
#include "asteroids.h"
#include "ufo.h"
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
char playerName[10] = "";
int framesPerSecond = 0;



void mainMenuInit()
{ 
    sprintf(str, "Hello");
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 2000, NULL, 3, &mainMenuTaskHandle);
    vTaskSuspend(mainMenuTaskHandle);
}

void mainMenuEnter()
{
    GDisplay* g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(mainMenuTaskHandle);
}

void mainMenuExit()
{
    GDisplay* g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(mainMenuTaskHandle);
    //xSemaphoreGive(g->mutex);
}

#define MAX_ASTEROID_COUNT_MENU 5

void mainMenuDrawTask(void *data)
{

    int asteroidCount = MAX_ASTEROID_COUNT_MENU;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_MENU] = {{0}};
    generateAsteroids(&asteroids, asteroidCount, asteroidCount, (pointf){0, 0}, 20);
    struct buttons buttons;
    
    //UFo
    struct ufo myufo;
    initUfo(&myufo);
    myufo.position.x = 63;
    myufo.position.y = 33;
    myufo.size = 2;
    
    //Drawing Menu
    int TextOffset = 30;
    int selectorPositionY = 60;
    int selectedOffsetX[4] = {30, 0, 0, 0};
    int selectedBool = 0;
    int selectedBool2 = 0;
    int selected = 0;
    //sprintf(str, "Asteroids %i", swidth);


    //Writing Name
    uint8_t writeName = 0;
    int nameChar = 65;
    //gdispImageOpenFile(&myImage, "sprites.png");
    // gdispImageClose(&myImage);

    gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
    // gdispImageClose(&myImage);
    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            drawAsteroids(&asteroids, asteroidCount, HTML2COLOR(0xb3b3b3));
            drawUfo(&myufo);

            //gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
            //gdispImageDraw(&titleImage, 30, 30, 210, 40, 0, 0);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    if (selected == 0)
                    {
                        xQueueSend(state_queue, &gameStateId, 0);
                    }
                    else if(selected == 3 && !writeName)
                    {
                        writeName = 1;
                        sprintf(playerName, nameChar);
                    }
                    else if(writeName = 1)
                    {
                        // sprintf(playerName,"Clicked");
                        writeName = 0;
                        // selected = 3;
                    }
                }
                
            }
            if(writeName)
            {
                if(buttons.joystick.y < -5)
                {
                    if(selectedBool2 == 0)
                    {
                        selectedBool2 = 1;
                        if(nameChar == 65)
                        {
                            nameChar = 57;
                        }
                        else if(nameChar == 48)
                        {
                            nameChar = 90;
                        }
                        else
                        {
                            nameChar --;
                        }
                        
                        
                    }  
                }
                else if (buttons.joystick.y > 5)
                {
                    if(selectedBool2 == 0)
                    {
                        selectedBool2 = 1;
                        if(nameChar == 90)
                        {
                            nameChar = 48;
                        }
                        else if (nameChar == 57)
                        {
                            nameChar = 65;
                        }
                        else
                        {
                           nameChar ++;
                        }
                    }
                }
                else if(buttons.joystick.x > 0)
                {
                    if(selectedBool2 == 0)
                    {
                        selectedBool2 = 1;
                        
                    }
                }
                else if(buttons.joystick.x < 0)
                {
                    if(selectedBool2 == 0)
                    {
                        selectedBool2 = 1;
                        sprintf(nameChar, strlen(nameChar), "%c", 48);
                    } 
                }
                else
                {
                    selectedBool2 = 0;
                }  
                if(buttons.B.risingEdge)
                {
                    sprintf(playerName + strlen(playerName), "A");
                } 
                sprintf(playerName, "%c", nameChar);   
            }

            if(buttons.joystick.y < 0 && selectorPositionY > 60 && !writeName)
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
            else if(buttons.joystick.y > 0 && selectorPositionY < 150 && !writeName)
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

            point pointsShip[] = {{0,0}, {0,16}, {20,8}};
            point pointsShipVertical[] = {{0,0},{-8,20},{8,20}};
            
            if(!writeName)
            {
                gdispFillConvexPoly(TextOffset, selectorPositionY, pointsShip, 3, White);
            }
            else
            {
                gdispFillConvexPoly(TextOffset + selectedOffsetX[3] + gdispGetStringWidth("Name: ", font16) + 5, 170, pointsShipVertical, 3, White);
            }
            

            sprintf(str, "Asteroids");
            gdispDrawString(TextOffset-5, 10, str, font32, White);

            sprintf(str, "Start Game");
            gdispDrawString(TextOffset+selectedOffsetX[0], 60, str, font16, White);
            
            sprintf(str, "Mode:");
            gdispDrawString(TextOffset+selectedOffsetX[1], 90, str, font16, White);
            int offset = gdispGetStringWidth(str,font16);

            sprintf(str, "  Single Player");
            gdispDrawString(TextOffset+offset+selectedOffsetX[1], 90, str, font16, White);

            sprintf(str, "High Score");
            gdispDrawString(TextOffset+selectedOffsetX[2], 120, str, font16, White);

            sprintf(str, "Name: ");
            gdispDrawString(TextOffset+selectedOffsetX[3], 150, str, font16, White);
            gdispDrawString(TextOffset + selectedOffsetX[3] + gdispGetStringWidth(str, font16), 150, playerName, font16, White);
            
            sprintf(str, "Frames %i", nameChar);
            gdispDrawString(DISPLAY_SIZE_X-100, DISPLAY_SIZE_Y-20, str, font16, White);

            // sprintf(str, "", swi)
        }
    }
}