#include "states/game.h"
#include "includes.h"
#include "asteroids.h"
#include "bullets.h"
#include "input.h"
#include "math.h"
#include "sm.h"
#include "uart.h"
#include "states/states.h"
#include "states/level_change_screen.h"
#include "stdlib.h"
#include "ufo.h"
#include "string.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"
#include "player.h"

TaskHandle_t drawTaskHandle;
TaskHandle_t generateAsteroidsHandle;
QueueHandle_t score_queue;
QueueHandle_t game_start_queue;

void gameInit()
{
    xTaskCreate(gameDrawTask, "gameDrawTask", 3000, NULL, 3, &drawTaskHandle);
    //XTaskCreate(generateAsteroid, "generateAsteroidsHandle", 3000, NULL, &generateAsteroidsHandle);
    vTaskSuspend(drawTaskHandle);
    score_queue = xQueueCreate(1, sizeof(struct player));
    game_start_queue = xQueueCreate(1, sizeof(struct gameStartInfo));
}

void gameEnter()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(drawTaskHandle);
    xTaskNotifyGive(drawTaskHandle);
}

void gameExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(drawTaskHandle);
}

void damagePlayer(struct player *player)
{
    if (player->health > 0)
        player->health -= 1;
    if (player->health <= 0)
    {
        struct changeScreenData changeScreen = {{0}};
        changeScreen.msWaitingTime = 2000;
        changeScreen.nextState = mainMenuStateId;
        changeScreen.showCountdown = 0;
        sprintf(changeScreen.Title, "Game over");
        sprintf(changeScreen.Subtext, "Score: %i", player->score);

        xQueueSend(levelChange_queue, &changeScreen, 0);
        xQueueSend(score_queue, player, 0);
        xQueueSend(state_queue, &levelChangeScreenId, 0);
    }
}

uint8_t checkWinCondition(struct asteroid asteroids[], int numAsteroids, struct player *player, struct ufo *ufo)
{
    if (ufo->isActive)
        return FALSE;

    for (int ai = 0; ai < numAsteroids; ai++)
    {
        struct asteroid *a = &asteroids[ai];
        if (a->isActive)
            return FALSE;
    }

    return TRUE;
}

void checkGameWin(struct asteroid asteroids[], int numAsteroids, struct player *player, struct ufo *ufo)
{
    if (checkWinCondition(asteroids, numAsteroids, player, ufo))
    {
        struct changeScreenData changeScreen = {{0}};
        changeScreen.msWaitingTime = 2000;
        changeScreen.nextState = mainMenuStateId;
        changeScreen.showCountdown = 0;
        sprintf(changeScreen.Title, "You Win!");
        sprintf(changeScreen.Subtext, "Score: %i", player->score);

        xQueueSend(levelChange_queue, &changeScreen, 0);
        xQueueSend(score_queue, player, 0);
        xQueueSend(state_queue, &levelChangeScreenId, 0);
    }
}

void checkCollisions(struct bullet bullets[], int numBullets, struct asteroid asteroids[], int numAsteroids, struct player *player, struct ufo *ufo)
{
    for (int ai = 0; ai < numAsteroids; ai++)
    {
        struct asteroid *a = &asteroids[ai];
        if (!a->isActive)
            continue;

        // Between player bullets and asteroids
        for (int bi = 0; bi < numBullets; bi++)
        {
            struct bullet *b = &bullets[bi];
            if (!b->isActive || b->type == FROM_UFO)
                continue;

            if (pointWithinCircle(a->position, a->radius, b->position))
            {
                // Collision
                player->score += 500;
                destroyAsteroid(asteroids, numAsteroids, ai);
                checkGameWin(asteroids, numAsteroids, player, ufo);
                b->isActive = 0;
            }
        }

        // Between player and asteroids
        if (cirlceTouchingCircle(a->position, a->radius, player->position, player->colliderRadius))
        {
            destroyAsteroid(asteroids, numAsteroids, ai);
            checkGameWin(asteroids, numAsteroids, player, ufo);
            damagePlayer(player);
        }
    }

    for (int bi = 0; bi < numBullets; bi++)
    {
        struct bullet *b = &bullets[bi];
        if (!b->isActive)
            continue;

        // Between bullets and ufo
        if (ufo->isActive && b->type == FROM_PLAYER && pointWithinCircle(ufo->position, ufo->size * 10, b->position))
        {
            player->score += 1000;
            b->isActive = 0;
            ufo->isActive = 0;
            checkGameWin(asteroids, numAsteroids, player, ufo);
        }

        // Between bullets and player
        if (b->type == FROM_UFO && pointWithinCircle(player->position, player->colliderRadius, b->position))
        {
            b->isActive = 0;
            damagePlayer(player);
        }
    }
}


void resetGame(struct player *player, struct ufo *ufo, struct asteroid *asteroids, size_t asteroidLength)
{
    player->health = 4;
    player->position = (pointf){DISPLAY_SIZE_X / 2.0, DISPLAY_SIZE_Y / 2.0};
    player->score = 0;
    player->speed = (pointf){0.0, 0.0};
    player->angleRad = 0;
    player->colliderRadius = 25;

    int initialAsteroidCount = 5;
    int asteroidsRadius = 20;

    //memset(asteroids, 0, asteroidLength);
    inactivateArray(asteroids, sizeof(struct asteroid), asteroidLength);
    generateAsteroids(asteroids, asteroidLength, initialAsteroidCount, (pointf){0, 0}, asteroidsRadius);
    spawnUfo(ufo, TRUE);
}

//void gfxMutexExit(gfxMutex *pmutex);

void gameDrawTask(void *data)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Buffer
    char str[100];
    struct buttons buttons;

    // Game Stats
    int maxLifes = 6;

    // asteroids
    int maxAsteroidCount = MAX_ASTEROID_COUNT_GAME;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME] = {{0}};

    // bullets
    int maxNumBullets = 10;
    struct bullet bullets[maxNumBullets];

    // player
    struct player player;

    // Ufo
    struct ufo ufo;

    uint8_t isMultiplayer;
    uint8_t isMaster;

    resetGame(&player, &ufo, &asteroids, sizeof(asteroids));
    while (1)
    {
        if (ulTaskNotifyTake(pdTRUE, 0) == 1)
        {
            resetGame(&player, &ufo, &asteroids, sizeof(asteroids));
            struct gameStartInfo gameStart;
            if (xQueueReceive(game_start_queue, &gameStart, 0) == pdTRUE)
            {
                isMultiplayer = gameStart.isMultiplayer;
                isMaster = gameStart.isMaster;
                strcpy(player.name, gameStart.name);
            }
        }

        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            //gdispClear(White);

            if (!isMultiplayer || isMaster)
            {
                updateAsteroids((struct asteroid *)&asteroids, maxAsteroidCount);
                checkCollisions(bullets, maxNumBullets, asteroids, maxAsteroidCount, &player, &ufo);

                if (isMultiplayer)
                {
                    // MASTER
                    // Send asteroids, bullets
                }
            }

            if (isMultiplayer)
            {
                // Receive/Send Player position
                sendFramePacket(&player, &asteroids, sizeof(asteroids));
                struct uartFramePacket framePacket;
                if (xQueueReceive(uartFramePacketQueue, &framePacket, 0) == pdTRUE)
                {
                    ufo.position = framePacket.playerPosition;
                    ufo.speed = framePacket.playerSpeed;
                    memcpy(asteroids, framePacket.asteroids, sizeof(asteroids));
                }
            }
            else
            {
                if (ufo.isActive)
                {
                    updateUfo(&ufo);
                    if (ufoShouldShoot(&ufo))
                    {
                        ufoShoot(&ufo, &player, &bullets, sizeof(bullets));
                    }
                }
            }

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    generateBullet(bullets, sizeof(bullets), player.angleRad, 5.0, 1.0, player.position, player.speed, FROM_PLAYER);
                }
                else if (buttons.D.risingEdge)
                {
                    struct changeScreenData changeScreen = {{0}};
                    
                    changeScreen.isPauseScreen = 1;
                    sprintf(changeScreen.Title, "Game Over");
                    sprintf(changeScreen.Subtext, "Score: %i", player.score);
                    changeScreen.nextState = mainMenuStateId;
                    

                    xQueueSend(levelChange_queue, &changeScreen, 0);
                    xQueueSend(state_queue, &levelChangeScreenId, 0);
                }
            }

            updateBullets(&bullets, maxNumBullets);
            updatePlayer(&player, buttons.joystick.x, buttons.joystick.y);

            // DRAWING

            drawAsteroids((struct asteroid *)&asteroids, maxAsteroidCount, White);
            drawPlayer(&player);
            drawUfo(&ufo, Red);
            drawBullets(&bullets, maxNumBullets);

            //Score counter on top
            sprintf(str, "%s : %i", player.name, player.score);
            gdispDrawString(10, 10, str, font12, White);
            point gameLifePoints[] = {{4, 0}, {0, 12}, {8, 12}};

            // Life indicator
            for (int i = 0; i < player.health; i++)
            {
                gdispFillConvexPoly(maxLifes * 12 - (i * 12), 30, gameLifePoints, 3, White);
            }
            for (int i = player.health; i < maxLifes; i++)
            {
                gdispDrawPoly(maxLifes * 12 - (i * 12), 30, gameLifePoints, 3, White);
            }

            // Debug view
            TickType_t nowTime = xTaskGetTickCount();
            float angleDeg = player.angleRad * 180 / M_PI;
            uint16_t fps = 1000 / (nowTime - xLastWakeTime);
            snprintf(str, 100, "FPS: %i, X: %i, Y: %i, Ag: %.2f, Sx: %.2f, Sy: %.2f, MP: %i", fps, buttons.joystick.x, buttons.joystick.y, angleDeg, player.speed.x, player.speed.y, isMultiplayer);
            gdispDrawString(DISPLAY_SIZE_X - gdispGetStringWidth(str, font1) - 10, DISPLAY_SIZE_Y - 20, str, font1, White);

            xLastWakeTime = nowTime;
            //gdispImageClose(&spriteSheet);
        }
    }
}
