#include "states/main_menu.h"
#include "states/states.h"
#include "includes.h"
#include "input.h"
#include "asteroids.h"
#include "ufo.h"
#include "sm.h"
#include "uart.h"
#include "states/game.h"
#include "states/level_change_screen.h"
#include "string.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"
#include "player.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ASTEROID_COUNT_MENU 5
#define HIGHSCORE_DISPLAY_COUNT 6

TaskHandle_t mainMenuTaskHandle;
SemaphoreHandle_t disconnectSemaphore;

//no neet to use Semaphores or Mutex
struct userScore userScoresSp[HIGHSCORE_DISPLAY_COUNT]; //Singleplayer
struct userScore userScoresMp[HIGHSCORE_DISPLAY_COUNT]; //Mulitplayer

char debugStr[40];

void mainMenuInit()
{
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 2000, NULL, 3, &mainMenuTaskHandle);
    disconnectSemaphore = xSemaphoreCreateBinary();
    vTaskSuspend(mainMenuTaskHandle);
}

void mainMenuEnter()
{
    struct player player;
    // sprintf(player.name, "Max");
    // player.score = 1005265;
    if (xQueueReceive(score_queue, &player, 0) == pdTRUE)
    {
        int i = 0;
        while (player.score < userScoresSp[i].score)
        {
            i++;
        }

        for (int j = HIGHSCORE_DISPLAY_COUNT - 1; j > i; j--)
        {
            userScoresSp[j].score = userScoresSp[j - 1].score;
            sprintf(userScoresSp[j].name, userScoresSp[j - 1].name);
        }
        sprintf(userScoresSp[i].name, player.name);
        userScoresSp[i].score = player.score;
        // sprintf(debugStr, "%i : %i : %i",player.score, userScoresSp[0].score, i);
    }
    else
    {
        sprintf(debugStr, "false");
    }

    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(mainMenuTaskHandle);
}

void mainMenuExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(mainMenuTaskHandle);
    //xSemaphoreGive(g->mutex);
}

void dispHighScore(uint8_t gameMode)
{
    char str[20];
    font_t myFont = font16;
    int highscorOffsetY = 70;

    if (gameMode == GAME_MODE_MP)
    {
        sprintf(str, "Highscore MP");
        gdispDrawString(LEFT_TEXT_MARGIN, 10, str, font32, White);
        for (int i = 0; i < HIGHSCORE_DISPLAY_COUNT; i++)
        {
            if (i == 0)
                myFont = font24;
            else if (i == 1)
                myFont = font20;
            else if (i == 2)
                myFont = font16;
            else
                myFont = font16;

            sprintf(str, "%i", i + 1);
            gdispDrawString(30, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, userScoresMp[i].name);
            gdispDrawString(60, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, "%i", userScoresMp[i].score);
            gdispDrawString(DISPLAY_SIZE_X - (int)(gdispGetStringWidth(str, myFont) + 20), highscorOffsetY + (i * 30), str, myFont, White);
        }
    }
    else
    {
        sprintf(str, "Highscore SP");
        gdispDrawString(LEFT_TEXT_MARGIN, 10, str, font32, White);
        for (int i = 0; i < HIGHSCORE_DISPLAY_COUNT; i++)
        {
            if (i == 0)
                myFont = font20;
            else if (i == 1)
                myFont = font20;
            else if (i == 2)
                myFont = font16;
            else
                myFont = font12;

            sprintf(str, "%i", i + 1);
            gdispDrawString(30, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, userScoresSp[i].name);
            gdispDrawString(60, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, "%i", userScoresSp[i].score);
            gdispDrawString(DISPLAY_SIZE_X - (int)(gdispGetStringWidth(str, myFont) + 20), highscorOffsetY + (i * 30), str, myFont, White);
        }
    }
}

void displayMenu(int selectorPositionY, uint8_t gameMode, uint8_t writeNameBool, int *selectedOffsetX, struct userNameInput *userName)
{
    point pointsShip[] = {{0, 0}, {0, 16}, {20, 8}};
    point pointsShipVertical[] = {{0, 0}, {-8, 20}, {8, 20}};
    char str[30];
    if (!writeNameBool)
    {
        gdispFillConvexPoly(LEFT_TEXT_MARGIN, selectorPositionY, pointsShip, 3, White);
    }
    else
    {
        gdispFillConvexPoly(LEFT_TEXT_MARGIN + selectedOffsetX[3] +
                        gdispGetStringWidth("Name: ", font16) + 5 + userName->cursorOffset,
                            170, pointsShipVertical, 3, White);
    }

    sprintf(str, "Asteroids");
    gdispDrawString(LEFT_TEXT_MARGIN - 5, 10, str, font32, White);

    sprintf(str, "Start Game");
    if (!writeNameBool)
    {
        gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[0] + gdispGetStringWidth(str, font16) + 10, 60, userName->name, font16, White);
    }
    gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[0], 60, str, font16, White);

    sprintf(str, "Mode:");
    gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[1], 90, str, font16, White);

    if (gameMode == GAME_MODE_SP)
        sprintf(str, "  Singleplayer");
    else if(gameMode == GAME_MODE_MP)
        sprintf(str, "  Multiplayer");
    else if(gameMode == GAME_MODE_GOD)
        sprintf(str, "  God Mode");
    else
        sprintf(str, "  Other Mode"); //Todo

    gdispDrawString(LEFT_TEXT_MARGIN + 47 + selectedOffsetX[1], 90, str, font16, White);

    sprintf(str, "High Score");
    gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[2], 120, str, font16, White);

    sprintf(str, "Name: ");
    gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[3], 150, str, font16, White);
    gdispDrawString(LEFT_TEXT_MARGIN + selectedOffsetX[3] + gdispGetStringWidth(str, font16), 150, userName->name, font16, White);
}

void writeName(struct buttons *buttons, struct userNameInput *userName)
{
    // int nameCharIndex = *_nameCharIndex;
    if (buttons->joystick.y < -5)
    {
        if (userName->lastJoystickPosition == 0)
        {
            userName->lastJoystickPosition = 1;
            if (userName->name[userName->charIndex] == 'A')
            {
                userName->name[userName->charIndex] = '9';
            }
            else if (userName->name[userName->charIndex] == '0')
            {
                userName->name[userName->charIndex] = 'Z';
            }
            else
            {
                userName->name[userName->charIndex]--;
            }
        }
    }
    else if (buttons->joystick.y > 5) //Move Down
    {
        if (userName->lastJoystickPosition == 0)
        {
            userName->lastJoystickPosition = 1;
            if (userName->name[userName->charIndex] == 'Z')
            {
                userName->name[userName->charIndex] = '0';
            }
            else if (userName->name[userName->charIndex] == '9')
            {
                userName->name[userName->charIndex] = 'A';
            }
            else
            {
                userName->name[userName->charIndex]++;
            }
        }
    }
    else if (buttons->joystick.x > 0) //Move Right
    {
        if (userName->lastJoystickPosition == 0)
        {
            userName->lastJoystickPosition = 1;
            if (userName->charIndex < 8)
            {
                if (userName->name[userName->charIndex] == 'i' || userName->name[userName->charIndex] == 'j')
                {
                    userName->cursorOffset += 2;
                }
                else
                {
                    userName->cursorOffset += 12;
                }
                userName->charIndex += 1;

                if (userName->name[userName->charIndex] == '\0')
                {
                    userName->name[userName->charIndex] = 'A';
                }
            }
        }
    }
    else if (buttons->joystick.x < 0) //Move Left
    {
        if (userName->lastJoystickPosition == 0)
        {
            userName->lastJoystickPosition = 1;
            if (userName->charIndex > 0)
            {
                if (userName->name[userName->charIndex] == 'i' || userName->name[userName->charIndex] == 'j')
                {
                    userName->cursorOffset -= 2;
                }
                else
                {
                    userName->cursorOffset -= 12;
                    if (userName->cursorOffset < 0)
                    {
                        userName->cursorOffset = 0;
                    }
                }
                userName->charIndex -= 1;
            }
        }
    }
    else
    {
        userName->lastJoystickPosition = 0;
    }

    if (buttons->B.risingEdge) //Delete last Character
    {
        int i = 0;
        while (userName->name[i] != '\0')
        {
            i++;
        }
        if (i > 1)
        {
            userName->name[i - 1] = '\0';
        }
        if (userName->charIndex == i - 1 && userName->charIndex > 0)
        {
            userName->cursorOffset -= 12;
            userName->charIndex--;
        }
    }
}

void disconnectTimerElapsed(TimerHandle_t xTimer)
{
    xSemaphoreGive(disconnectSemaphore);
}

void startGame(uint8_t gameMode, uint8_t isMaster, char *name)
{
    struct gameStartInfo gameStart;
    gameStart.isMaster = isMaster;
    gameStart.level = 1;
    gameStart.mode = gameMode;

    if (name[0] == '\0')
    {
        // int i = 0;
        sprintf(name, "PLAYER");
    }

    strcpy(gameStart.name, name);

    struct changeScreenData changeScreenData = {{0}};
    sprintf(changeScreenData.Title, "Level 1");
    sprintf(changeScreenData.Subtext, name);
    changeScreenData.msWaitingTime = COUNTDOWN_START * 1000;
    changeScreenData.showCountdown = 1;
    changeScreenData.nextState = gameStateId;

    xQueueSend(levelChange_queue, &changeScreenData, 0);
    xQueueSend(game_start_queue, &gameStart, 0);
    xQueueSend(state_queue, &levelChangeScreenId, 0);
}

void mainMenuDrawTask(void *data)
{
    int asteroidCount = MAX_ASTEROID_COUNT_MENU;
    struct asteroid asteroids[MAX_ASTEROID_COUNT_MENU] = {{0}};
    generateAsteroids(&asteroids, sizeof(asteroids), asteroidCount, (pointf){0, 0}, 20);
    struct buttons buttons;

    char str[100];

    //UFo
    struct ufo ufo = {{0}};
    ufo.position.x = 63;
    ufo.position.y = 24;
    ufo.showHealt = 0;
    ufo.speed.x = 0.4;
    ufo.speed.y = 0.1;
    ufo.size = 3;

    // ufo.health = 20;
    

    //Drawing Menu
    int selectorPositionY = 60;
    int selectedOffsetX[4] = {30, 0, 0, 0};
    int selectedBool = 0;
    int selected = 0;

    //Writing Name
    struct userNameInput userName = {{0}};

    //writing name
    userName.cursorOffset = 0;
    uint8_t writeNameBool = 0;

    //hight score
    uint8_t showHighScoreBool = 0;

    //Cheats 
    uint8_t gameMode = 0; //mode 0: sp, mode 1: mp, mode 2: cheat, mode 3: open;

    //multiplayer mode
    uint8_t isMuliPlayerBool = 0;
    uint8_t otherUserConnected = 0;
    uint8_t isMaster = 1;
    TimerHandle_t disconnectTimer;
    disconnectTimer = xTimerCreate("disconnectTimer", pdMS_TO_TICKS(500), pdTRUE, NULL, disconnectTimerElapsed);

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            drawAsteroids(&asteroids, asteroidCount, Gray);
            updateAsteroids(&asteroids, asteroidCount);
            drawUfo(&ufo, Grey);
            updateUfo(&ufo);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    if (selected == 0)
                    {
                        // Start Game
                        if (gameMode == GAME_MODE_MP) //Multiplayer
                        {
                            sendGameInvitation(FALSE, userName.name);
                        }
                        else
                        {
                            startGame(gameMode, isMaster, userName.name);
                        }
                    }
                    else if (selected == 1)
                    {
                        if(gameMode == 2)
                            gameMode = 0;
                        else
                            gameMode ++;
                        // if(gameMode == 1)
                        //     isMuliPlayerBool = 1;
                        // else
                        //     isMuliPlayerBool = 0;
                    }

                    else if (selected == 2 && !showHighScoreBool)
                    {
                        showHighScoreBool = 1;
                    }
                    else if (selected == 3 && !writeNameBool)
                    {
                        writeNameBool = 1;
                        if (userName.name[0] == '\0')
                        {
                            userName.name[0] = 65;
                        }
                    }
                    else if (writeNameBool)
                    {
                        writeNameBool = 0;
                    }
                    else if (showHighScoreBool)
                    {
                        showHighScoreBool = 0;
                    }
                }

                if(!showHighScoreBool)
                {
                    if (writeNameBool)
                    {
                        writeName(&buttons, &userName);
                    }

                    if (buttons.joystick.y < 0 && !writeNameBool) //UP
                    {
                        if (selectedBool == 0)
                        {
                            if (selected == 0)
                            {
                                selected = 3;
                                selectedOffsetX[0] = 0;
                            }
                            else
                            {
                                selected--;
                                selectedOffsetX[selected + 1] = 0;
                            }
                            selectedBool = 1;
                        }
                    }
                    else if (buttons.joystick.y > 0 && !writeNameBool) //DOWN
                    {
                        if (selectedBool == 0)
                        {
                            if (selected == 3)
                            {
                                selected = 0;
                                selectedOffsetX[3] = 0;
                            }
                            else
                            {
                                selected++;
                                selectedOffsetX[selected - 1] = 0;
                            }
                            selectedBool = 1;
                        }
                    }
                    else
                    {
                        selectedBool = 0;
                    }
                    selectedOffsetX[selected] = 30;
                    selectorPositionY = 60 + (selected * 30);
                }
                // sprintf(str, "", swi)
            }

            // Continuously send handshake if multiplayer is selected
            if (gameMode == GAME_MODE_MP)
            {
                sendHandshake(isMaster);
            }

            // When handshake received
            struct uartHandshakePacket handshakePacket;
            if (xQueueReceive(uartHandshakeQueue, &handshakePacket, 0) == pdTRUE)
            {
                xTimerReset(disconnectTimer, 0);
                isMaster = !handshakePacket.fromMaster;
                otherUserConnected = TRUE;
                // isMuliPlayerBool = TRUE;
                gameMode = GAME_MODE_MP;
            }

            // When disconnected
            if (xSemaphoreTake(disconnectSemaphore, 0) == pdTRUE)
            {
                otherUserConnected = FALSE;
            }

            // When other user started game
            struct uartGameInvitePacket invitePacket;
            if (xQueueReceive(uartInviteQueue, &invitePacket, 0) == pdTRUE)
            {
                if (!invitePacket.isAck)
                {
                    sendGameInvitation(TRUE, userName.name);
                }
                startGame(isMuliPlayerBool, isMaster, userName.name);
            }

            //Display Highscore
            if (showHighScoreBool)
            {
                dispHighScore(gameMode);
            }
            else //Drawing Menu
            {
                displayMenu(selectorPositionY, gameMode ,writeNameBool, &selectedOffsetX, &userName);
                if (otherUserConnected)
                {
                    sprintf(str, "Connected");
                    gdispDrawString(DISPLAY_SIZE_X - 130, DISPLAY_SIZE_Y - 20, str, font16, White);
                }

                // gdispDrawString(DISPLAY_SIZE_X - 130, DISPLAY_SIZE_Y - 20, debugStr, font16, White);
            }
        }
    }
}