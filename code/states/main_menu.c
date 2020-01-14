#include "states/main_menu.h"
#include "states/states.h"
#include "includes.h"
#include "input.h"
#include "asteroids.h"
#include "ufo.h"
#include "sm.h"
#include "uart.h"
#include "states/game.h"
#include "string.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ASTEROID_COUNT_MENU 5

TaskHandle_t mainMenuTaskHandle;
struct userScore userScores[10];
char debugStr[40];

void mainMenuInit()
{
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 2000, NULL, 3, &mainMenuTaskHandle);
    vTaskSuspend(mainMenuTaskHandle);
}

void mainMenuEnter()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(mainMenuTaskHandle);

    struct userScore tempUser;
    struct player player;
    // sprintf(player.name, "Max");
    // player.score = 1005265;
    if (xQueueReceive(score_queue, &player, 0) == pdTRUE)
    {
        sprintf(debugStr, "%i", player.score);
        for (int i = 0; i < 10; i++)
        {
            sprintf(userScores[i].name, "");
            userScores[i].score = 0;
        }
        sprintf(userScores[0].name, "Adam");
        userScores[0].score = 100000;

        int i = 0;
        while (player.score < userScores[i].score)
        {
            i++;
        }
        sprintf(userScores[i].name, player.name);
        userScores[i].score = player.score;
    }
    else
    {
        sprintf(debugStr, "false");
    }
}

void mainMenuExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(mainMenuTaskHandle);
    //xSemaphoreGive(g->mutex);
}

void dispHighScore(int TextOffset)
{
    char str[100];
    sprintf(str, "Highscore");
    gdispDrawString(TextOffset - 5, 10, str, font32, White);
    font_t myFont;
    int highscorOffsetY = 70;

    for (int i = 0; i < 10; i++)
    {
        if (i == 0)
            myFont = font24;
        else if (i == 1)
            myFont = font20;
        else if (i == 2)
            myFont = font16;
        else
            myFont = font12;
        sprintf(str, "%i", i + 1);
        gdispDrawString(30, highscorOffsetY + (i * 30), str, myFont, White);
        sprintf(str, userScores[i].name);
        gdispDrawString(60, highscorOffsetY + (i * 30), str, myFont, White);
        sprintf(str, "%i", userScores[i].score);
        gdispDrawString(DISPLAY_SIZE_X - (int)(gdispGetStringWidth(str, myFont) + 20), highscorOffsetY + (i * 30), str, myFont, White);
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
            if (userName->charIndex < 10)
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
        if (i > 0)
        {
            userName->name[i - 1] = '\0';
        }
        if (userName->charIndex == i - 1 && userName->charIndex > 1)
        {
            userName->cursorOffset -= 12;
            userName->charIndex--;
        }
    }
}

void dispMenu()
{
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

    userName.cursorOffset = 0;
    uint8_t writeNameBool = 0;

    //hight score
    uint8_t showHighScoreBool = 0;

    //multiplayer mode
    uint8_t isMuliPlayerBool = 0;
    uint8_t otherUserConnected = 0;
    uint8_t isMaster = 1;

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
                        struct gameStartInfo gameStart;
                        gameStart.isMultiplayer = isMuliPlayerBool;
                        gameStart.isMaster = TRUE;
                        strcpy(gameStart.name, userName.name);

                        xQueueSend(game_start_queue, &gameStart, 0);
                        xQueueSend(state_queue, &levelChangeScreenId, 0);
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
                    dispHighScore(TextOffset);
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

            // Send master handshake if multiplayer is selected
            if (isMuliPlayerBool)
            {
                sendHandshake(TRUE);
            }

            struct uartHandshakePacket handshakePacket;
            // When handshake received
            if (xQueueReceive(uartHandshakeQueue, &handshakePacket, 0) == pdTRUE)
            {
                isMaster = !handshakePacket.isMaster;
                otherUserConnected = TRUE;
                if (!isMaster)
                {
                    sendHandshake(isMaster);
                }
            }

            point pointsShip[] = {{0, 0}, {0, 16}, {20, 8}};
            point pointsShipVertical[] = {{0, 0}, {-8, 20}, {8, 20}};

            if (!writeNameBool)
            {
                gdispFillConvexPoly(TextOffset, selectorPositionY, pointsShip, 3, White);
            }
            else
            {
                gdispFillConvexPoly(TextOffset + selectedOffsetX[3] +
                    gdispGetStringWidth("Name: ", font16) + 5 +  userName.cursorOffset,
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
        }
    }
}