#include "states/game.h"
#include "includes.h"
#include "input.h"
#include "math.h"

TaskHandle_t drawTaskHandle;

void gameInit()
{
    xTaskCreate(gameDrawTask, "gameDrawTask", 3000, NULL, 3, &drawTaskHandle);
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

gdispImage spriteSheet;
void displayShip(int x, int y, uint8_t thrustOn) 
{
    if (thrustOn)
        gdispImageDraw(&spriteSheet, x, y, 13, 18, 15, 14);
    else
        gdispImageDraw(&spriteSheet, x, y, 13, 18, 0, 14);
}

void gameDrawTask(void* data)
{
    int i = 0;
    struct buttons buttons;
    char str[50];
    gdispImageOpenFile(&spriteSheet, "sprites.bmp");
    gdispImageCache(&spriteSheet);

    float shipX = 100;
    float shipY = 100;

    float speedShipX = 0;
    float speedShipY = 0;

    float maxSpeedX = 2;
    float maxSpeedY = 2;

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                sprintf(str, "X: %i, Y: %i, C: %i, Sx: %f, Sy: %f", buttons.joystick.x, buttons.joystick.y, i++, speedShipX, speedShipY);	
            }
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20 , str, font1, White);

            uint8_t thrustOn = buttons.joystick.x != 0 || buttons.joystick.y != 0;
            speedShipX += buttons.joystick.x / 2000.0;
            if (speedShipX > maxSpeedX) speedShipX = maxSpeedX;
            if (speedShipX < -maxSpeedX) speedShipX = -maxSpeedX;

            speedShipY += buttons.joystick.y / 2000.0;
            if (speedShipY > maxSpeedY) speedShipY = maxSpeedY;
            if (speedShipY < -maxSpeedY) speedShipY = -maxSpeedY;

            displayShip((int)shipX, (int)shipY, thrustOn);

            shipX += speedShipX;
            shipY += speedShipY;

            //gdispImageClose(&spriteSheet);
        }
    }
}

/*void generateAsteroid(int x, int y, int lineCount) {
    for (int i=0; i<lineCount, i++) 
    {
        float d = i - floor
    }
}*/