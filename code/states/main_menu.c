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
    struct userScore tempUser;
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

        for(int j = HIGHSCORE_DISPLAY_COUNT-1; j > i; j--)
        {
            userScoresSp[j].score = userScoresSp[j-1].score;
            sprintf(userScoresSp[j].name,userScoresSp[j-1].name);
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

void dispHighScore(int TextOffset, uint8_t isMulitpayer)
{
    char str[100];
    font_t myFont = font16;
    int highscorOffsetY = 70;

    if (isMulitpayer)
    {
        sprintf(str, "Highscore MP");
        gdispDrawString(TextOffset - 5, 10, str, font32, White);
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
        gdispDrawString(TextOffset - 5, 10, str, font32, White);
        for (int i = 0; i < HIGHSCORE_DISPLAY_COUNT; i++)
        {
            /* if (i == 0)
                myFont = font24;
            else if (i == 1)
                myFont = font20;
            else if (i == 2)
                myFont = font16;
            else
                myFont = font12; */

            sprintf(str, "%i", i + 1);
            gdispDrawString(30, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, userScoresSp[i].name);
            gdispDrawString(60, highscorOffsetY + (i * 30), str, myFont, White);
            sprintf(str, "%i", userScoresSp[i].score);
            gdispDrawString(DISPLAY_SIZE_X - (int)(gdispGetStringWidth(str, myFont) + 20), highscorOffsetY + (i * 30), str, myFont, White);
        }
    }
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

void startGame(uint8_t isMultiplayer, uint8_t isMaster, char *name)
{
    struct gameStartInfo gameStart;
    gameStart.isMultiplayer = isMultiplayer;
    gameStart.isMaster = isMaster;

    if(name[0] == '\0')
    {
        // int i = 0;
        sprintf(name, "PLAYER"); 
    }

    strcpy(gameStart.name, name);

    struct changeScreenData changeScreenData = {{0}};
    sprintf(changeScreenData.Title, "Level 1");
    sprintf(changeScreenData.Subtext, name);
    changeScreenData.msWaitingTime = COUNTDOWN_START*1000;
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
    char playerName[10] = "";

    //UFo
    int updateSpeedTime = 0;
    struct ufo myufo;
    myufo.position.x = 63;
    myufo.position.y = 33;
    myufo.speed.x = 0.4;
    myufo.speed.y = 0.1;
    myufo.size = 2;

    //Drawing Menu
    int TextOffset = 30;
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
    uint8_t gameMode = 0;

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
            drawUfo(&myufo, Grey);
            updateUfo(&myufo);
            updateSpeedTime++;
            if (updateSpeedTime > 50)
            {
                updateSpeedTime = 0;

                myufo.speed.y += (randRange(-4, 4)) / 13.0;
                myufo.speed.x += (randRange(-4, 4)) / 13.0;
            }
            // xTaskGetTickCount

            //gdispImageDraw(&myImage, 30, 30, 28, 25, 0, 56);
            //gdispImageDraw(&titleImage, 30, 30, 210, 40, 0, 0);

            if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
            {
                if (buttons.C.risingEdge)
                {
                    if (selected == 0)
                    {
                        // Start Game
                        if (isMuliPlayerBool)
                        {
                            sendGameInvitation(FALSE, userName.name);
                        }
                        else
                        {
                            startGame(isMuliPlayerBool, isMaster, userName.name);
                        }
                    }
                    else if (selected == 1)
                    {
                        isMuliPlayerBool = !isMuliPlayerBool;
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
                        // sprintf(playerName, nameChar);
                    }
                    else if (writeNameBool)
                    {
                        // sprintf(playerName,"Clicked");
                        writeNameBool = 0;
                        // selected = 3;
                    }
                    else if (showHighScoreBool)
                    {
                        showHighScoreBool = 0;
                    }
                }

                if (showHighScoreBool)
                {
                    dispHighScore(TextOffset, isMuliPlayerBool);
                }
                else
                {
                    if (writeNameBool)
                    {
                        writeName(&buttons, &userName);
                    }

                    if (buttons.joystick.y < 0 && selectorPositionY > 60 && !writeNameBool)
                    {
                        if (selectedBool == 0)
                        {
                            selectorPositionY -= 30;
                            selected--;
                            selectedOffsetX[selected] = 30;
                            selectedOffsetX[selected + 1] = 0;
                            selectedBool = 1;
                        }
                    }
                    else if (buttons.joystick.y > 0 && selectorPositionY < 150 && !writeNameBool)
                    {
                        if (selectedBool == 0)
                        {
                            selectorPositionY += 30;
                            selected++;
                            selectedOffsetX[selected] = 30;
                            selectedOffsetX[selected - 1] = 0;
                            selectedBool = 1;
                        }
                    }
                    else
                    {
                        selectedBool = 0;
                    }
                }
                // sprintf(str, "", swi)
            }

            // Continuously send handshake if multiplayer is selected
            if (isMuliPlayerBool)
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
                isMuliPlayerBool = TRUE;
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
        
            if (showHighScoreBool)
            {
                dispHighScore(TextOffset, isMuliPlayerBool);
            }
            else
            {
                // Drawing
                point pointsShip[] = {{0, 0}, {0, 16}, {20, 8}};
                point pointsShipVertical[] = {{0, 0}, {-8, 20}, {8, 20}};

                if (!writeNameBool)
                {
                    gdispFillConvexPoly(TextOffset, selectorPositionY, pointsShip, 3, White);
                }
                else
                {
                    gdispFillConvexPoly(TextOffset + selectedOffsetX[3] +
                                            gdispGetStringWidth("Name: ", font16) + 5 + userName.cursorOffset,
                                        170,
                                        pointsShipVertical, 3, White);
                }

                sprintf(str, "Asteroids");
                gdispDrawString(TextOffset - 5, 10, str, font32, White);

                sprintf(str, "Start Game");
                if (!writeNameBool)
                {
                    gdispDrawString(TextOffset + selectedOffsetX[0] + gdispGetStringWidth(str, font16) + 10, 60, userName.name, font16, White);
                }
                gdispDrawString(TextOffset + selectedOffsetX[0], 60, str, font16, White);

                sprintf(str, "Mode:");
                gdispDrawString(TextOffset + selectedOffsetX[1], 90, str, font16, White);
                int offset = gdispGetStringWidth(str, font16);

                if (isMuliPlayerBool)
                    sprintf(str, "  Multiplayer");
                else
                    sprintf(str, "  Singleplayer");
                gdispDrawString(TextOffset + offset + selectedOffsetX[1], 90, str, font16, White);

                sprintf(str, "High Score");
                gdispDrawString(TextOffset + selectedOffsetX[2], 120, str, font16, White);

                sprintf(str, "Name: ");
                gdispDrawString(TextOffset + selectedOffsetX[3], 150, str, font16, White);
                gdispDrawString(TextOffset + selectedOffsetX[3] + gdispGetStringWidth(str, font16), 150, userName.name, font16, White);

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