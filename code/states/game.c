#include "states/game.h"
#include "includes.h"
#include "asteroids.h"
#include "bullets.h"
#include "input.h"
#include "math.h"
#include "sm.h"
#include "states/states.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"

TaskHandle_t drawTaskHandle;
TaskHandle_t generateAsteroidsHandle;

void gameInit()
{
    xTaskCreate(gameDrawTask, "gameDrawTask", 3000, NULL, 3, &drawTaskHandle);
    //XTaskCreate(generateAsteroid, "generateAsteroidsHandle", 3000, NULL, &generateAsteroidsHandle);
    vTaskSuspend(drawTaskHandle);
}

void gameEnter() 
{
    GDisplay* g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(drawTaskHandle);
}

void gameExit()
{
    GDisplay* g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
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


void destroyAsteroid(struct asteroid asteroids[], int numAsteroids, int index)
{
    struct asteroid* a = &asteroids[index];
    a->isActive = 0;
    int radius = a->radius - 10;

    if (radius > 0)
        generateAsteroids(asteroids,  numAsteroids * sizeof(struct asteroid), 2, a->position, radius);
}

void checkCollisions(struct bullet bullets[], int numBullets, struct asteroid asteroids[], int numAsteroids, struct player* player)
{

    for (int ai=0; ai<numAsteroids; ai++)
    {
        struct asteroid* a = &asteroids[ai];
        if (!a->isActive) continue;

        // Between bullets and asteroids
        for (int bi=0; bi<numBullets; bi++)
        {
            struct bullet* b = &bullets[bi];
            if (!b->isActive) continue;

            if (pointWithinCircle(a->position, a->radius, b->position))
            {
                // Collision
                player->score += 500;
                destroyAsteroid(asteroids, numAsteroids, ai);
                b->isActive = 0;
            }
        }

        // Between player and asteroids
        if (cirlceTouchingCircle(a->position, a->radius, player->position, player->colliderRadius))
        {
            destroyAsteroid(asteroids, numAsteroids, ai);
            if(player->health > 0)
                player->health -= 1;
            if (player->health <= 0)  
            {
                xQueueSend(state_queue, &mainMenuStateId, 0);
            }
        }

    }
}

//void gfxMutexExit(gfxMutex *pmutex);
#define MAX_ASTEROID_COUNT_GAME 20

void gameDrawTask(void* data)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char str[100];    

    //Game Stats
    int gameStartLifes = 10;

    struct buttons buttons;

    //all about asteroid
    int maxAsteroidCount = MAX_ASTEROID_COUNT_GAME;
    int initialAsteroidCount = 7;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME] = {{0}};
    generateAsteroids(&asteroids, sizeof(asteroids), initialAsteroidCount, (pointf){0, 0}, 20);

    // bullets
    int maxNumBullets = 10;
    struct bullet bullets[maxNumBullets];

    // player
    float shipMaxSpeed = 2;
    float lastAngleRad = 0;
    struct player player = {0, {100, 100}, {0, 0}, 20, 5};

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            checkCollisions(bullets, maxNumBullets, asteroids, maxAsteroidCount, &player);
            //draw asteroids
            drawAsteroids(&asteroids, maxAsteroidCount, White);
            updateAsteroids(&asteroids, maxAsteroidCount);

            uint8_t thrustOn = buttons.joystick.x != 0 || buttons.joystick.y != 0;

            
            if (thrustOn) 
            {
                float angleRad = atan2f(-buttons.joystick.y, buttons.joystick.x);
                float t = 0.1;

                lastAngleRad = lerp_angle(lastAngleRad, angleRad, t);
            }

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    generateBullet(bullets, sizeof(bullets), lastAngleRad, player.position, player.speed);
                }
            }

            float angle = lastAngleRad * 180 / M_PI;

            // FPS counter
			TickType_t nowTime = xTaskGetTickCount();
			uint16_t fps = 1000/ (nowTime - xLastWakeTime);
            snprintf(str, 100, "FPS: %i, X: %i, Y: %i, Ag: %.2f, Sx: %.2f, Sy: %.2f", fps, buttons.joystick.x, buttons.joystick.y, angle, i++, player.speed.x, player.speed.y);	
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20 , str, font1, White);

            for (int i=0; i<maxNumBullets; i++)
            {
                struct bullet* b = &bullets[i];
                if (b->isActive)
                {
                    gdispFillCircle(b->position.x, b->position.y, 3, Red);

                    b->position.x += b->speed.x;
                    b->position.y += b->speed.y;

                    if (b->position.x > DISPLAY_SIZE_X ||
                        b->position.y > DISPLAY_SIZE_Y ||
                        b->position.x < 0 ||
                        b->position.y < 0)
                    {
                        b->isActive = 0;
                    }
                }
            }

            player.speed.x += buttons.joystick.x / 2000.0;
            if (player.speed.x > shipMaxSpeed) player.speed.x = shipMaxSpeed;
            if (player.speed.x < -shipMaxSpeed) player.speed.x = -shipMaxSpeed;

            player.speed.y += buttons.joystick.y / 2000.0;
            if (player.speed.y > shipMaxSpeed) player.speed.y = shipMaxSpeed;
            if (player.speed.y < -shipMaxSpeed) player.speed.y = -shipMaxSpeed;

            int pointCount = 6;
            int flameLength = max(abs(buttons.joystick.x), abs(buttons.joystick.y)) * 12 / 127.0;

            point points[] = {{-8,0}, {8,0}, {0,20}, {-5, 0}, {5, 0}, {0, -flameLength-5}};
            for (int i = 0; i<pointCount; i++)
            {
                int newX = rotatePointX(points[i].x, points[i].y, lastAngleRad - M_PI/2);
                int newY = rotatePointY(points[i].x, points[i].y, lastAngleRad - M_PI/2);
                points[i] = (point){newX, newY};
            }

            gdispFillConvexPoly(player.position.x, player.position.y, points, 3, White);
            if (thrustOn)
                gdispFillConvexPoly(player.position.x, player.position.y, points + 3, 3, Orange);
            

            //Score counter on top

            sprintf(str, "%s : %i", player.name, player.score);
            gdispDrawString(10, 10, str, font12, White);
            point gameLifePoints[] = {{4,0}, {0,12}, {8,12}};

            for(int i = 0; i < player.health; i++)
            {
                gdispFillConvexPoly(gameStartLifes*12-(i*12), 30, gameLifePoints, 3, White);
            }
            for(int i = player.health; i < gameStartLifes; i++)
            {
                gdispDrawPoly(gameStartLifes*12-(i*12), 30, gameLifePoints, 3, White);
            }
                

            //Loosig a life
            if(buttons.B.fallingEdge && player.health > 0)
            {
                player.health --;
            }
            //gdispDrawThickLine(points[0].x + shipX, points[0].y + shipY, points[1]. x + shipX, points[1].y + shipY, White, 4, 1);
            //gdispBlitArea((int)shipX, (int)shipY, 60, 60, surface1);
            //displayShip((int)shipX, (int)shipY, thrustOn);

            player.position.x += player.speed.x;
            player.position.y += player.speed.y;

            if (player.position.x > DISPLAY_SIZE_X) player.position.x = 0;
            if (player.position.y > DISPLAY_SIZE_Y) player.position.y = 0;
            if (player.position.x < 0) player.position.x = DISPLAY_SIZE_X;
            if (player.position.y < 0) player.position.y = DISPLAY_SIZE_Y;

            xLastWakeTime = nowTime;
            //gdispImageClose(&spriteSheet);
        }
    }
}
