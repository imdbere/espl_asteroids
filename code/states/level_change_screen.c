#include "includes.h"
#include "input.h"
#include "sm.h"
#include "states/states.h"
#include "states/level_change_screen.h"
#include "states/game.h"
#include "stdlib.h"
#include "asteroids.h"
#include "ufo.h"
#include "uart.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"

TaskHandle_t levelChangeScreenTaskHandle;
QueueHandle_t levelChange_queue;

struct changeScreenData changeScreenData;

void levelChangeScreenInit()
{
    levelChange_queue = xQueueCreate(1, sizeof(struct changeScreenData));
    xTaskCreate(levelChangeScreenDraw, "levelChangeScreenDraw", 2000, NULL, 3, &levelChangeScreenTaskHandle);
    vTaskSuspend(levelChangeScreenTaskHandle);
}

void levelChangeScreenEnter()
{
    xTaskNotifyGive(levelChangeScreenTaskHandle);
    xQueueReceive(levelChange_queue, &changeScreenData, 0);
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(levelChangeScreenTaskHandle);
}

void levelChangeScreenExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(levelChangeScreenTaskHandle);
}

void levelChangeScreenDraw(void *data)
{

    uint8_t resetBool = 0;
    char str[25];
    uint32_t lastTimeUpdate;
    struct buttons buttons;

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            if (!changeScreenData.isPauseScreen)
            {
                if (resetBool == 0)
                {
                    lastTimeUpdate = xTaskGetTickCount();
                    resetBool = 1;
                }
                sprintf(str, changeScreenData.Title);
                gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font32) / 2), 40, str, font32, White);
                sprintf(str, changeScreenData.Subtext);
                gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font24) / 2), 90, str, font24, White);
                if (changeScreenData.showCountdown)
                {
                    sprintf(str, "start in: %i", ((changeScreenData.msWaitingTime - (xTaskGetTickCount() - lastTimeUpdate)) / 1000) + 1);
                    gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font20) / 2), 120, str, font20, White);
                }

                if ((xTaskGetTickCount() - lastTimeUpdate) >= changeScreenData.msWaitingTime)
                {
                    resetBool = 0;
                    gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font16) / 2), DISPLAY_SIZE_Y / 2 + 50, str, font16, White);
                    xQueueSend(state_queue, &changeScreenData.nextState, 0);
                }
            }
            else
            {
                sprintf(str, "Pause");
                gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font32) / 2), 40, str, font32, White);
                sprintf(str, "Press D to continue");
                gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font24) / 2), 90, str, font24, White);
                sprintf(str, "Press A to exit");
                gdispDrawString((DISPLAY_SIZE_X / 2) - (gdispGetStringWidth(str, font24) / 2), 130, str, font24, White);

                if (xQueueReceive(ButtonQueue, &buttons, 0) == pdTRUE)
                {
                    if(buttons.D.risingEdge)
                    {
                        xQueueSend(state_queue, &gameStateId, 0);
                        resetDisconnectTimer();
                        sendResume();
                    }
                    else if(buttons.A.risingEdge)
                    {   
                        changeScreenData.isPauseScreen = 0;
                        resetDisconnectTimer();
                        sendExit();
                    }
                }

                struct uartPausePacket pausePacket;
                if (xQueueReceive(uartPauseQueue, &pausePacket, 0) == pdTRUE)
                {
                    if (pausePacket.mode == PAUSE_MODE_RESUME)
                        xQueueSend(state_queue, &gameStateId, 0);
                    else if (pausePacket.mode == PAUSE_MODE_EXIT)
                        changeScreenData.isPauseScreen = 0;

                    resetDisconnectTimer();
                }
            }
        }
    }
}