#include "states/mainMenu.h"
#include "includes.h"
#include "input.h"

static gdispImage spriteSheet;

void gameInit()
{

}

void gameEnter() 
{
    
}

void gameExit()
{

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

void gameRun()
{
    if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
    {
        gdispClear(Black);
        gdispImageOpenFile(&spriteSheet, "sprites.png");
        //gdispClear(White);

        if (xQueueReceive(ButtonQueue, &buttons, 1) == pdTRUE)
        {
            sprintf(str, "X: %i, Y: %i, C: %i", buttons.joystick.x, buttons.joystick.y, i++);	
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20 , str, font1, White);
        }

        uint8_t thrustOn = buttons.joystick.x > 0 || buttons.joystick.y > 0;
        displayShip(100, 100, thrustOn);
        //gdispImageClose(&spriteSheet);
    }
}