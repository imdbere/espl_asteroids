#include "states/game.h"
#include "includes.h"
#include "input.h"

static gdispImage spriteSheet;
TaskHandle_t drawTaskHandle;

void gameInit()
{
    gdispImageOpenFile(&spriteSheet, "sprites.png");
    xTaskCreate(gameDrawTask, "gameDrawTask", 200, NULL, 3, &drawTaskHandle);
    vTaskSuspend(drawTaskHandle);
}

void gameEnter() 
{
    vTaskResume(drawTaskHandle);
}

void gameExit()
{
    vTaskSuspend(drawTaskHandle);
}

void displayShip(int x, int y, uint8_t thrustOn) 
{
    if (thrustOn)
        gdispImageDraw(&spriteSheet, x, y, 13, 18, 15, 14);
    else
        gdispImageDraw(&spriteSheet, x, y, 13, 18, 0, 14);
}

int i = 0;
static struct buttons buttons;
static char str[20];

void gameDrawTask(void* data)
{
    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                sprintf(str, "X: %i, Y: %i, C: %i", buttons.joystick.x, buttons.joystick.y, i++);	
            }
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20 , str, font1, White);

            uint8_t thrustOn = buttons.joystick.x > 0 || buttons.joystick.y > 0;
            displayShip(100, 100, thrustOn);
            //gdispImageClose(&spriteSheet);
        }
    }
}