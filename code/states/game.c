#include "states/game.h"
#include "includes.h"
#include "asteroids.h"
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

int rotatePointX (int x, int y, float angle) 
{
    return (int) (cos(angle)*(x ) - sin(angle)*(y )) ;
}

int rotatePointY (int x, int y, float angle) 
{
    return (int) (sin(angle)*(x ) + cos(angle)*(y )) ;
}

/*float lerp_angle(float a, float b, float t)
{https://www.google.com/
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

void generateBullet(struct bullet* b, pointf pos, pointf playerSpeed, float angle)
{
    float speedMagnitude = 5.0;
    float playerSpeedInheritanceAmount = 1;

    b->isActive = 1;
    b->position = pos;
    b->speed.x = -speedMagnitude * sin(angle) + playerSpeed.x * playerSpeedInheritanceAmount;
    b->speed.y = speedMagnitude * cos(angle) + playerSpeed.y * playerSpeedInheritanceAmount;
}

pointf addPoints(pointf p1, pointf p2)
{
    pointf p = {p1.x + p2.x, p1.y + p2.y};
    return p;
}

float square(float nr)
{
    return nr * nr;
}

uint8_t cirlceTouchingCircle(pointf pos1, float rad1, pointf pos2, float rad2)
{
    return square(pos1.x - pos2.x) + square(pos1.y - pos2.y) < square(rad1 + rad2);
}

uint8_t pointWithinCircle(pointf circlePos, float circleRadius, pointf point)
{
    return cirlceTouchingCircle(circlePos, circleRadius, point, 0);
}

void destroyAsteroid(struct asteroid asteroids[], int numAsteroids, int index)
{
    struct asteroid* a = &asteroids[index];
    a->isActive = 0;
    int radius = a->radius - 10;

    if (radius > 0)
        generateAsteroids(asteroids, numAsteroids, 2, a->position, radius);
}

void __attribute__((optimize("O0"))) checkCollisions(struct bullet bullets[], int numBullets, struct asteroid asteroids[], int numAsteroids, struct player* player)
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
                destroyAsteroid(asteroids, numAsteroids, ai);
                b->isActive = 0;
            }
        }

        // Between player and asteroids
        if (cirlceTouchingCircle(a->position, a->radius, player->position, player->colliderRadius))
        {
            destroyAsteroid(asteroids, numAsteroids, ai);
            player->health -= 1;
            if (player->health == 0)  
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
    //all about asteroid
    int maxAsteroidCount = MAX_ASTEROID_COUNT_GAME;
    int initialAsteroidCount = 7;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME] = {{0}};

    generateAsteroids(&asteroids, maxAsteroidCount, initialAsteroidCount, (pointf){0, 0}, 20);

    //Game Stats
    int gameStartLifes = 10;
    int gameScore = 0;

    int i = 0;
    struct buttons buttons;
    char str[100];
    gdispImageOpenFile(&spriteSheet, "sprites.bmp");
    gdispImageCache(&spriteSheet);

    struct player player = {0, {100, 100}, {0, 0}, 20, 5};

    float shipMaxSpeed = 2;
    //float maxSpeedY = 2;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    GDisplay* pixmap = gdispPixmapCreate(40, 40);
    pixel_t* surface = gdispPixmapGetBits(pixmap);

    displayShip(pixmap, 10, 10, 0);

    GDisplay* pixmap1 = gdispPixmapCreate(60, 60);
    pixel_t* surface1 = gdispPixmapGetBits(pixmap);

    float angle = 45 * M_PI / 180;
    float lastAngleRad = 0;

    int maxNumBullets = 10;
    struct bullet bullets[maxNumBullets];
    //List* bulletList = makelist();
    
    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            checkCollisions(bullets, maxNumBullets, asteroids, maxAsteroidCount, &player);
            //draw asteroids
            drawAsteroids(&asteroids, maxAsteroidCount, White);

            uint8_t thrustOn = buttons.joystick.x != 0 || buttons.joystick.y != 0;

            
            if (thrustOn) 
            {
                float angleRad = atan2f(-buttons.joystick.x, buttons.joystick.y);
                float t = 0.1;

                lastAngleRad = lerp_angle(lastAngleRad, angleRad, t);
            }

            float angle = lastAngleRad * 180 / M_PI;

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    /*struct bullet* newBullet = malloc(sizeof(struct bullet));
                    generateBullet(newBullet, player.position, lastAngleRad);
                    add(newBullet, bulletList);*/

                    // Search free array space
                    int i =0;
                    for (i=0; i<maxNumBullets; i++)
                    {
                        if (!bullets[i].isActive)
                            break;
                    }

                    if (i != maxNumBullets)
                    {
                        struct bullet* newBullet = &bullets[i];
                        generateBullet(newBullet, player.position, player.speed, lastAngleRad);
                    }
                }
            }

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
                int newX = rotatePointX(points[i].x, points[i].y, lastAngleRad);
                int newY = rotatePointY(points[i].x, points[i].y, lastAngleRad);
                points[i] = (point){newX, newY};
            }

            gdispFillConvexPoly(player.position.x, player.position.y, points, 3, White);
            if (thrustOn)
                gdispFillConvexPoly(player.position.x, player.position.y, points + 3, 3, Orange);
            

            //Score counter on top

            sprintf(str, "%.5i", gameScore++);
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

void generateAsteroid(void* data) 
{
    
}