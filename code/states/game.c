#include "states/game.h"
#include "includes.h"
#include "asteroids.h"
#include "bullets.h"
#include "input.h"
#include "math.h"
#include "sm.h"
#include "uart.h"
#include "states/states.h"
#include "states/main_menu.h"
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
    xTaskCreate(gameDrawTask, "gameDrawTask", 5000, NULL, 3, &drawTaskHandle);
    //XTaskCreate(generateAsteroid, "generateAsteroidsHandle", 3000, NULL, &generateAsteroidsHandle);
    vTaskSuspend(drawTaskHandle);
    score_queue = xQueueCreate(1, sizeof(struct userScore));
    game_start_queue = xQueueCreate(1, sizeof(struct gameStartInfo));
}

void gameEnter()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    // xTaskNotifyGive(drawTaskHandle);
    vTaskResume(drawTaskHandle);
}

void gameExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(drawTaskHandle);
}

void damagePlayer(struct player *player, uint8_t gameMode)
{
    if (gameMode != GAME_MODE_GOD)
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
            
            struct userScore userScore = {{0}};
            userScore.gameMode = gameMode;
            userScore.score = player->score;
            sprintf(userScore.name, player->name);

            xQueueSend(levelChange_queue, &changeScreen, 0);
            xQueueSend(score_queue, &userScore, 0);
            xQueueSend(state_queue, &levelChangeScreenId, 0);
        }
    }
}

void damageUfo(struct ufo *ufo)
{
    ufo->health -= 1;
    if (ufo->health <= 0)
    {
        ufo->isActive = 0;
    }
}

uint8_t checkWinCondition(struct asteroid asteroids[], size_t asteroidsLength, struct player *player, struct ufo *ufos, size_t ufosLength, uint8_t isMultiplayer)
{
    return 
        isEmpty(ufos, sizeof(struct ufo), ufosLength) &&
        (isEmpty(asteroids, sizeof(struct asteroid), asteroidsLength) || isMultiplayer);
        
}

void checkGameWin(struct asteroid asteroids[], size_t asteroidsLength, struct player *player, struct ufo *ufos, size_t ufosLength, uint8_t gameMode)
{
    if (checkWinCondition(asteroids, asteroidsLength, player, ufos, ufosLength, gameMode == GAME_MODE_MP))
    {
        struct changeScreenData changeScreen = {{0}};
        struct gameStartInfo gameStart = {{0}};

        //NEXT LEVEL
        if (gameMode != GAME_MODE_MP && player->level < LEVEL_COUNT)
        {
            player->level++;
            changeScreen.msWaitingTime = COUNTDOWN_NEXT_LEVEL * 1000;
            changeScreen.nextState = gameStateId;
            changeScreen.showCountdown = 1;
            gameStart.level = player->level;
            gameStart.mode = gameMode;
            sprintf(gameStart.name, player->name);
            sprintf(changeScreen.Title, "Level %i", player->level);

            xQueueSend(levelChange_queue, &changeScreen, 0);
            xQueueSend(game_start_queue, &gameStart, 0);
            xQueueSend(state_queue, &levelChangeScreenId, 0);
        }
        else
        {
            changeScreen.msWaitingTime = 2000;
            changeScreen.nextState = mainMenuStateId;
            changeScreen.showCountdown = 0;
            sprintf(changeScreen.Title, "You Win!");
            sprintf(changeScreen.Subtext, "Score: %i", player->score);

            struct userScore userScore = {{0}};
            userScore.gameMode = gameMode;
            userScore.score = player->score;
            sprintf(userScore.name, player->name);


            xQueueSend(levelChange_queue, &changeScreen, 0);
            xQueueSend(score_queue, &userScore, 0);
            xQueueSend(state_queue, &levelChangeScreenId, 0);
        }
    }
}

void handleCollision(struct uartCollisionPacket col, struct bullet bullets[], struct asteroid asteroids[], size_t asteroidsLength, struct player *player, struct ufo *ufos, size_t ufosLength, uint8_t gameMode)
{
    uint8_t isMultiplayer = gameMode == GAME_MODE_MP;

    if (col.collider1 == COLL_ASTEROID)
    {
        if (col.collider2 == COLL_BULLET)
        {
            // Collision
            if (asteroids[col.collider1Id].radius < RADIUS_BIG_ASTEROID)
                player->score += POINTS_DESTROY_SMALL_ASTEROID;
            else
                player->score += POINTS_DESTROY_BIG_ASTEROID;

            //Get free Life
            if ((player->score - player->scoreOld) > POINTS_FOR_HEALTH)
            {
                player->scoreOld = player->score;
                player->health += 1;
            }

            destroyAsteroid(asteroids, asteroidsLength, col.collider1Id);
            bullets[col.collider2Id].isActive = 0;
        }
        else if (col.collider2 == COLL_PLAYER)
        {
            destroyAsteroid(asteroids, asteroidsLength, col.collider1Id);
            damagePlayer(player, gameMode);
        }
        else if (col.collider2 == COLL_UFO)
        {
            destroyAsteroid(asteroids, asteroidsLength, col.collider1Id);
            damageUfo(&ufos[0]);
        }

        checkGameWin(asteroids, asteroidsLength, player, ufos, ufosLength, isMultiplayer);
    }

    if (col.collider1 == COLL_BULLET)
    {
        if (col.collider2 == COLL_UFO)
        {
            player->score += POINTS_DESTROY_UFO;
            bullets[col.collider1Id].isActive = 0;
            damageUfo(&ufos[col.collider2Id]);
            checkGameWin(asteroids, asteroidsLength, player, ufos, ufosLength, isMultiplayer);
        }
        if (col.collider2 == COLL_PLAYER)
        {
            bullets[col.collider1Id].isActive = 0;
            damagePlayer(player, gameMode);
        }
    }
}

int checkCollisions(struct uartCollisionPacket *collisions, struct bullet bullets[], int numBullets, struct asteroid asteroids[], int numAsteroids, struct player *player, struct ufo *ufos, uint8_t maxUfoCount, uint8_t gameMode)
{
    int currentCollision = 0;

    // Asteroids
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
                struct uartCollisionPacket *col = &collisions[currentCollision++];
                col->collider1 = COLL_ASTEROID;
                col->collider2 = COLL_BULLET;
                col->collider1Id = ai;
                col->collider2Id = bi;
            }
        }

        // Between player and asteroids
        if (cirlceTouchingCircle(a->position, a->radius, player->position, player->colliderRadius))
        {
            struct uartCollisionPacket *col = &collisions[currentCollision++];
            col->collider1 = COLL_ASTEROID;
            col->collider2 = COLL_PLAYER;
            col->collider1Id = ai;
        }

        // Between player ufo and asteroids (in case of multiplayer)
        if (ufos[0].collidesWithAsteroids && cirlceTouchingCircle(a->position, a->radius, ufos[0].position, ufos[0].colliderRadius))
        {
            struct uartCollisionPacket *col = &collisions[currentCollision++];
            col->collider1 = COLL_ASTEROID;
            col->collider2 = COLL_UFO;
            col->collider1Id = ai;
        }

    }

    //Bullets
    for (int bi = 0; bi < numBullets; bi++)
    {
        struct bullet *b = &bullets[bi];
        if (!b->isActive)
            continue;

        // Between bullets and ufo
        for (int ui = 0; ui < maxUfoCount; ui++)
        {
            struct ufo *ufo = &ufos[ui];
            if (ufo->isActive && b->type == FROM_PLAYER && pointWithinCircle(ufo->position, ufo->colliderRadius, b->position))
            {
                struct uartCollisionPacket *col = &collisions[currentCollision++];
                col->collider1 = COLL_BULLET;
                col->collider2 = COLL_UFO;
                col->collider1Id = bi;
                col->collider2Id = ui;
            }
        }

        // Between bullets and player
        if (b->type != FROM_PLAYER && pointWithinCircle(player->position, player->colliderRadius, b->position))
        {
            struct uartCollisionPacket *col = &collisions[currentCollision++];
            col->collider1 = COLL_BULLET;
            col->collider2 = COLL_PLAYER;
            col->collider1Id = bi;
        }
    }

    return currentCollision;
}

void resetGame(struct player *player, struct ufo *ufos, uint8_t maxUfoCount, struct asteroid *asteroids, size_t asteroidLength, uint8_t isMultiplayer, uint8_t isMaster, uint8_t level)
{
    player->health = INITIAL_HEALTH_COUNT;

    if (isMultiplayer)
    {
        spawnUfo(&ufos[0], TRUE);
        ufos[0].maxHealth = UFO_MAX_LIFES_MP;
        ufos[0].health = INITIAL_HEALTH_COUNT;
    }
    else
    {
        for (int i = 0; i < maxUfoCount; i++)
        {
            ufos[i].isActive = 0;
        }
    }

    player->position = (pointf){DISPLAY_SIZE_X / 2.0, DISPLAY_SIZE_Y / 2.0};
    if (level == 1)
        player->score = 0;
    player->scoreOld = 0;
    player->speed = (pointf){0.0, 0.0};
    player->angleRad = 0;
    player->level = level;

    player->colliderRadius = RADIUS_COLLIDER;
    if (isMultiplayer)
    {
        ufos[0].colliderRadius = RADIUS_COLLIDER;
        ufos[0].collidesWithAsteroids = 1;
    }


    int initialAsteroidCount = isMultiplayer 
        ? MAX_ASTEROID_COUNT_MP 
        : INITIAL_ASTEROID_COUNT + ((level - 1) * ADD_ASTEROID_PER_LEVEL);

    int asteroidsRadius = RADIUS_BIG_ASTEROID;

    //inactivateArray(asteroids, sizeof(struct asteroid), asteroidLength);
    //inactivateArray(bullets, sizeof(struct asteroid), asteroidLength);
    if (!isMultiplayer || isMaster)
        generateAsteroids(asteroids, asteroidLength, initialAsteroidCount, (pointf){0, 0}, asteroidsRadius);
}

//void gfxMutexExit(gfxMutex *pmutex);

void gameDrawTask(void *data)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Buffer
    char str[100];
    struct buttons buttons;

    // Game Stats
    int maxLifes = MAX_HEALTH_COUNT;

    // asteroids
    int maxAsteroidCount = MAX_ASTEROID_COUNT_GAME;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_GAME] = {{0}};

    // bullets
    int maxNumBullets = MAX_BULLET_COUNT;
    struct bullet bullets[MAX_BULLET_COUNT] = {{0}};

    // player
    struct player player;

    // Ufo
    int maxUfoCount = UFO_MAX_COUNT;
    int ufosInField = 0;
    struct ufo ufos[UFO_MAX_COUNT] = {{0}};

    //Game Mode
    uint8_t gameMode = 0;
    uint8_t isMultiplayer = 0;
    uint8_t isMaster = 0;

    struct uartCollisionPacket collisions[MAX_SIMULTANEOUS_COLLISIONS];
    uint8_t sentDisconnected = 0;

    //resetGame(&player, &ufo, &asteroids, sizeof(asteroids), isMultiplayer);
    while (1)
    {

        struct gameStartInfo gameStart;
        if (xQueueReceive(game_start_queue, &gameStart, 0) == pdTRUE)
        {
            gameMode = gameStart.mode;
            if (gameMode == GAME_MODE_MP)
                isMultiplayer = 1;
            isMaster = gameStart.isMaster;
            strcpy(player.name, gameStart.name);
            player.level = gameStart.level;

            memset(asteroids, 0, sizeof(asteroids));
            memset(bullets, 0, sizeof(bullets));

            sentDisconnected = 0;
            ufosInField = 0;
            resetGame(&player, &ufos, maxUfoCount, &asteroids, sizeof(asteroids), isMultiplayer, isMaster, gameStart.level);

            if (isMultiplayer)
            {
                if (isMaster)
                {
                    sendGameSetup(&asteroids, MAX_ASTEROID_COUNT_MP * sizeof(struct asteroid));
                }
                else
                {
                    struct uartGameSetupPacket gameSetup;
                    if (xQueueReceive(uartGameSetupQueue, &gameSetup, 100) == pdTRUE)
                    {
                        memcpy(&asteroids, &gameSetup.asteroids, MAX_ASTEROID_COUNT_MP * sizeof(struct asteroid));
                    }
                }
                xSemaphoreTake(disconnectSemaphore, 0);
            }
        }

        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            struct uartFramePacket framePacket = {{0}};

            // Button detection
            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                // Shoot new bullet
                if (buttons.C.risingEdge)
                {
                    struct bullet *newBullet = generateBullet(bullets, sizeof(bullets), player.angleRad, 5.0, 1.0, player.position, player.speed, FROM_PLAYER);
                    if (isMultiplayer)
                    {
                        memcpy(&framePacket.newBullet, newBullet, sizeof(struct bullet));
                    }
                }
                // Pause Screen
                else if (buttons.D.risingEdge)
                {
                    struct changeScreenData changeScreen = {{0}};

                    changeScreen.isPauseScreen = 1;
                    sprintf(changeScreen.Title, "Game Over");
                    sprintf(changeScreen.Subtext, "Score: %i", player.score);
                    changeScreen.nextState = mainMenuStateId;
                    changeScreen.msWaitingTime = 2000;

                    xQueueSend(levelChange_queue, &changeScreen, 0);
                    xQueueSend(state_queue, &levelChangeScreenId, 0);
                }
            }

            // Updating stuff that is required in singleplayer
            // and gets calculated by master in multiplayer
            if (!isMultiplayer || isMaster)
            {
                int numCollisions = checkCollisions(collisions, bullets, maxNumBullets, asteroids, maxAsteroidCount, &player, &ufos, maxUfoCount, gameMode);
                for(int i=0; i<numCollisions; i++)
                {
                    struct uartCollisionPacket *col = &collisions[i];
                    if (isMultiplayer)
                    {
                        struct uartCollisionPacket mpCol = *col;
                        // Switch Player <-> UFO for multiplayer
                        if (col->collider2 == COLL_PLAYER) mpCol.collider2 = COLL_UFO;
                        if (col->collider2 == COLL_UFO) mpCol.collider2 = COLL_PLAYER;

                        // Create common seed for next asteroid generation
                        mpCol.nextAsteroidSeed = rand();
                        srand(mpCol.nextAsteroidSeed);
                        sendCollisionPacket(&mpCol);
                    }

                    handleCollision(*col, bullets, asteroids, sizeof(asteroids), &player, ufos, sizeof(ufos), gameMode); 
                }
            }

            if (isMultiplayer)
            {
                // Receive/Send Player position
                framePacket.playerPosition = player.position;
                framePacket.playerSpeed = player.speed;

                sendFramePacket(&framePacket);

                if (xQueueReceive(uartFramePacketQueue, &framePacket, 0) == pdTRUE)
                {
                    ufos[0].position = framePacket.playerPosition;
                    ufos[0].speed = framePacket.playerSpeed;
                    // Generate bullet triggered by other player
                    if (framePacket.newBullet.isActive)
                    {
                        struct bullet *b = (struct bullet *)searchForFreeSpace(bullets, sizeof(struct bullet), 1);
                        memcpy(b, &framePacket.newBullet, sizeof(struct bullet));
                        b->type = FROM_PLAYER_UFO;
                    }
                    //memcpy(asteroids, framePacket.asteroids, sizeof(asteroids));
                }

                if (!isMaster)
                {
                    struct uartCollisionPacket collisionPacket;
                    while (xQueueReceive(uartCollosionPacketQueue, &collisionPacket, 0) == pdTRUE)
                    {
                        srand(collisionPacket.nextAsteroidSeed);
                        handleCollision(collisionPacket, bullets, asteroids, sizeof(asteroids), &player, ufos, sizeof(ufos), gameMode);
                        //memcpy(asteroids, framePacket.asteroids, sizeof(asteroids));
                    }
                }

                // When disconnected
                if (xSemaphoreTake(disconnectSemaphore, 0) == pdTRUE && !sentDisconnected)
                {
                    struct changeScreenData changeScreen = {{0}};

                    changeScreen.msWaitingTime = 2000;
                    changeScreen.nextState = mainMenuStateId;
                    changeScreen.showCountdown = 0;
                    changeScreen.isPauseScreen = 0;
                    sprintf(changeScreen.Title, "Disconnected!");
                    sprintf(changeScreen.Subtext, "");

                    xQueueSend(levelChange_queue, &changeScreen, 0);
                    xQueueSend(score_queue, &player, 0);
                    xQueueSend(state_queue, &levelChangeScreenId, 0);

                    sentDisconnected = TRUE;
                }

            }
            else
            {
                if (ufosInField < UFO_MAX_COUNT && player.score > UFO_SPAWN_MIN_SCORE)
                {
                    if (spawnUfoRandom(&ufos, sizeof(ufos)))
                        ufosInField++;
                }

                updateUfo(ufos, maxUfoCount);
                for (int ui = 0; ui < maxUfoCount; ui++)
                {
                    if (ufos[ui].isActive && ufoShouldShoot())
                    {
                        ufoShoot(&ufos[ui], &player, &bullets, sizeof(bullets));
                    }
                }
            }

            updateAsteroids((struct asteroid *)&asteroids, maxAsteroidCount);
            updateBullets(&bullets, maxNumBullets);
            updatePlayer(&player, buttons.joystick.x, buttons.joystick.y);

            // DRAWING
            gdispClear(Black);

            drawAsteroids((struct asteroid *)&asteroids, maxAsteroidCount, White);
            drawPlayer(&player);

            drawUfo(&ufos, maxUfoCount);

            drawBullets(&bullets, maxNumBullets);
            drawPlayer(&player);

            //Score counter on top
            sprintf(str, "%s : %i Level: %i Mode: %i", player.name, player.score, player.level, gameMode);
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
