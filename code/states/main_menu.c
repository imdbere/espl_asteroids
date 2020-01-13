#include "states/main_menu.h"
#include "states/states.h"
#include "includes.h"
#include "input.h"
#include "asteroids.h"
#include "ufo.h"
#include "sm.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

TaskHandle_t mainMenuTaskHandle;
QueueHandle_t name_queue;
struct userScore userScores[10];
char debugStr[40];
void mainMenuInit()
{
    xTaskCreate(mainMenuDrawTask, "mainMenuDrawTask", 2000, NULL, 3, &mainMenuTaskHandle);
    vTaskSuspend(mainMenuTaskHandle);
    name_queue = xQueueCreate(1, 20);
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
    if(xQueueReceive(score_queue, &player, 0) == pdTRUE)
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

#define MAX_ASTEROID_COUNT_MENU 5

void setName(void)
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
    // sprintf(playerName, "");

    //UFo
    int updateSpeedTime = 0;
    struct ufo myufo;
    //initUfo(&myufo);
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
    int selectedBool2 = 0;
    int selected = 0;
    //sprintf(str, "Asteroids %i", swidth);

    //Writing Name
    uint8_t writeName = 0;
    int nameShipOffset = 0;
    int nameCharIndex = 0;

    //hight score
    uint8_t showHighScore = 0;

    //multiplayer mode
    uint8_t isMuliPlayer = 0;

    //gdispImageOpenFile(&myImage, "sprites.png");
    // gdispImageClose(&myImage);

    //gdispImageOpenFile(&titleImage, "mainTextAsteroids.png");
    // gdispImageClose(&myImage);
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
                        xQueueSend(name_queue, &playerName, 0);

                        xQueueSend(state_queue, &gameStateId, 0);
                    }
                    else if (selected == 1)
                    {
                        isMuliPlayer = !isMuliPlayer;
                    }

                    else if (selected == 2 && !showHighScore)
                    {
                        showHighScore = 1;
                    }
                    else if (selected == 3 && !writeName)
                    {
                        writeName = 1;
                        if (playerName[0] == '\0')
                        {
                            playerName[0] = 65;
                        }
                        // sprintf(playerName, nameChar);
                    }
                    else if (writeName)
                    {
                        // sprintf(playerName,"Clicked");
                        writeName = 0;
                        // selected = 3;
                    }
                    else if (showHighScore)
                    {
                        showHighScore = 0;
                    }
                }
                else if (buttons.B.risingEdge)
                {
                    if (writeName)
                    {
                        int i = 0;
                        while (playerName[i] != '\0')
                        {
                            i++;
                        }
                        if (i > 0)
                        {
                            playerName[i - 1] = '\0';
                        }
                        if (nameCharIndex == i - 1 && nameCharIndex > 1)
                        {
                            nameShipOffset -= 12;
                            nameCharIndex--;
                        }
                    }
                    else if (showHighScore)
                    {
                        showHighScore = 0;
                    }
                }
            }

            if (showHighScore)
            {
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
            else
            {
                if (writeName)
                {
                    if (buttons.joystick.y < -5)
                    {
                        if (selectedBool2 == 0)
                        {
                            selectedBool2 = 1;
                            if (playerName[nameCharIndex] == 'A')
                            {
                                playerName[nameCharIndex] = '9';
                            }
                            else if (playerName[nameCharIndex] == '0')
                            {
                                playerName[nameCharIndex] = 'Z';
                            }
                            else
                            {
                                playerName[nameCharIndex]--;
                            }
                        }
                    }
                    else if (buttons.joystick.y > 5) //Move Down
                    {
                        if (selectedBool2 == 0)
                        {
                            selectedBool2 = 1;
                            if (playerName[nameCharIndex] == 90)
                            {
                                playerName[nameCharIndex] = 48;
                            }
                            else if (playerName[nameCharIndex] == 57)
                            {
                                playerName[nameCharIndex] = 65;
                            }
                            else
                            {
                                playerName[nameCharIndex]++;
                            }
                        }
                    }
                    else if (buttons.joystick.x > 0) //Move Right
                    {
                        if (selectedBool2 == 0)
                        {
                            selectedBool2 = 1;
                            if (nameCharIndex < 10)
                            {
                                if (playerName[nameCharIndex] == 'i' || playerName[nameCharIndex] == 'j')
                                {
                                    nameShipOffset += 2;
                                }
                                else
                                {
                                    nameShipOffset += 12;
                                }

                                nameCharIndex += 1;
                                if (playerName[nameCharIndex] == '\0')
                                {
                                    playerName[nameCharIndex] = 'A';
                                }
                            }
                        }
                    }
                    else if (buttons.joystick.x < 0) //Move Left
                    {
                        if (selectedBool2 == 0)
                        {
                            selectedBool2 = 1;
                            if (nameCharIndex > 0)
                            {
                                if (playerName[nameCharIndex] == 'i' || playerName[nameCharIndex] == 'j')
                                {
                                    nameShipOffset -= 2;
                                    nameCharIndex -= 1;
                                }
                                else
                                {
                                    nameShipOffset -= 12;
                                    if (nameShipOffset < 0)
                                    {
                                        nameShipOffset = 0;
                                    }
                                    nameCharIndex -= 1;
                                }
                            }
                        }
                    }
                    else
                    {
                        selectedBool2 = 0;
                    }
                }

                if (buttons.joystick.y < 0 && selectorPositionY > 60 && !writeName)
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
                else if (buttons.joystick.y > 0 && selectorPositionY < 150 && !writeName)
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

                point pointsShip[] = {{0, 0}, {0, 16}, {20, 8}};
                point pointsShipVertical[] = {{0, 0}, {-8, 20}, {8, 20}};

                if (!writeName)
                {
                    gdispFillConvexPoly(TextOffset, selectorPositionY, pointsShip, 3, White);
                }
                else
                {
                    gdispFillConvexPoly(TextOffset + selectedOffsetX[3] +
                                            gdispGetStringWidth("Name: ", font16) + 5 + nameShipOffset,
                                        170,
                                        pointsShipVertical, 3, White);
                }

                sprintf(str, "Asteroids");
                gdispDrawString(TextOffset - 5, 10, str, font32, White);

                sprintf(str, "Start Game");
                if (!writeName)
                {
                    gdispDrawString(TextOffset + selectedOffsetX[0] + gdispGetStringWidth(str, font16) + 10, 60, playerName, font16, White);
                }
                gdispDrawString(TextOffset + selectedOffsetX[0], 60, str, font16, White);

                sprintf(str, "Mode:");
                gdispDrawString(TextOffset + selectedOffsetX[1], 90, str, font16, White);
                int offset = gdispGetStringWidth(str, font16);

                if (isMuliPlayer)
                    sprintf(str, "  Multiplayer");
                else
                    sprintf(str, "  Singleplayer");
                gdispDrawString(TextOffset + offset + selectedOffsetX[1], 90, str, font16, White);

                sprintf(str, "High Score");
                gdispDrawString(TextOffset + selectedOffsetX[2], 120, str, font16, White);

                sprintf(str, "Name: ");
                gdispDrawString(TextOffset + selectedOffsetX[3], 150, str, font16, White);
                gdispDrawString(TextOffset + selectedOffsetX[3] + gdispGetStringWidth(str, font16), 150, playerName, font16, White);

                sprintf(str, "Frames %s", debugStr);
                gdispDrawString(DISPLAY_SIZE_X - 130, DISPLAY_SIZE_Y - 20, str, font16, White);
            }

            // sprintf(str, "", swi)
        }
    }
}