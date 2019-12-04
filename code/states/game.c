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
void displayShip(GDisplay* g, int x, int y, uint8_t thrustOn) 
{
    if (thrustOn)
        gdispGImageDraw(g, &spriteSheet, x, y, 13, 18, 15, 14);
    else
        gdispGImageDraw(g, &spriteSheet, x, y, 13, 18, 0, 14);
}

int rotatePointX (int x, int y, float angle) 
{
    return (int) (cos(angle)*(x ) - sin(angle)*(y )) ;
}

int rotatePointY (int x, int y, float angle) 
{
    return (int) (sin(angle)*(x ) + cos(angle)*(y )) ;
}

/*float lerp_angle(float a, float b, float t)
{
    if (abs(a-b) >= M_PI)
        if (a > b):
            a = normalize_angle(a) - 2.0 * M_PI
        else
            b = normalize_angle(b) - 2.0 * M_PI
    return lerp(a, b, t)
}*/
float floatMod(float a, float b)
{
    return (a - b * floor(a / b));
}

float short_angle_dist(float from, float to)
{
    float max_angle = M_PI * 2;
    float difference = floatMod(to - from, max_angle);
    return floatMod(2 * difference, max_angle) - difference;
}

float lerp_angle(float from, float to, float weight)
{
    return from + short_angle_dist(from, to) * weight;
}

void gameDrawTask(void* data)
{
    int i = 0;
    struct buttons buttons;
    char str[100];
    gdispImageOpenFile(&spriteSheet, "sprites.bmp");
    gdispImageCache(&spriteSheet);

    float shipX = 100;
    float shipY = 100;

    float speedShipX = 0;
    float speedShipY = 0;

    float maxSpeedX = 2;
    float maxSpeedY = 2;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    GDisplay* pixmap = gdispPixmapCreate(40, 40);
    pixel_t* surface = gdispPixmapGetBits(pixmap);

    displayShip(pixmap, 10, 10, 0);

    GDisplay* pixmap1 = gdispPixmapCreate(60, 60);
    pixel_t* surface1 = gdispPixmapGetBits(pixmap);

    float angle = 45 * M_PI / 180;
    float lastAngleRad = 0;

    for(int y = 0; y < 20; y++)
    {
    	for(int x = 0; x < 20; x++)
        {
            surface1[rotatePointY(x, y, angle) * 40 + rotatePointX(x, y, angle)] = surface[y * 60 + x];
        }
    }

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                
            }

            uint8_t thrustOn = buttons.joystick.x != 0 || buttons.joystick.y != 0;

            
            if (thrustOn) 
            {
                float angleRad = atan2f(-buttons.joystick.x, buttons.joystick.y);
                float t = 0.1;

                lastAngleRad = lerp_angle(lastAngleRad, angleRad, t);
            }

            float angle = lastAngleRad * 180 / M_PI;

            // FPS counter
			TickType_t nowTime = xTaskGetTickCount();
			uint16_t fps = 1000/ (nowTime - xLastWakeTime);
            snprintf(str, 100, "FPS: %i, X: %i, Y: %i, Ag: %.2f, Sx: %.2f, Sy: %.2f", fps, buttons.joystick.x, buttons.joystick.y, angle, i++, speedShipX, speedShipY);	
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20 , str, font1, White);

            speedShipX += buttons.joystick.x / 2000.0;
            if (speedShipX > maxSpeedX) speedShipX = maxSpeedX;
            if (speedShipX < -maxSpeedX) speedShipX = -maxSpeedX;

            speedShipY += buttons.joystick.y / 2000.0;
            if (speedShipY > maxSpeedY) speedShipY = maxSpeedY;
            if (speedShipY < -maxSpeedY) speedShipY = -maxSpeedY;

            point points[3] = {{0,0}, {10,0}, {5,15}};
            for (int i = 0; i<3; i++)
            {
                int newX = rotatePointX(points[i].x, points[i].y, lastAngleRad);
                int newY = rotatePointY(points[i].x, points[i].y, lastAngleRad);
                points[i] = (point){newX, newY};
            }

            gdispDrawPoly(shipX, shipY, points, 3, White);
            //gdispBlitArea((int)shipX, (int)shipY, 60, 60, surface1);
            //displayShip((int)shipX, (int)shipY, thrustOn);

            shipX += speedShipX;
            shipY += speedShipY;

            if (shipX > DISPLAY_SIZE_X) shipX = 0;
            if (shipY > DISPLAY_SIZE_Y) shipY = 0;
            if (shipX < 0) shipX = DISPLAY_SIZE_X;
            if (shipY < 0) shipY = DISPLAY_SIZE_Y;

            xLastWakeTime = nowTime;
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