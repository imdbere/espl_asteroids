#include "includes.h"
#include "input.h"
#include "sm.h"
#include "states/states.h"
#include "states/level_change_screen.h"
#include "stdlib.h"
#include "asteroids.h"
#include "ufo.h"
#include "src/gdisp/gdisp_driver.h"
#include "src/gos/gos_freertos.h"


TaskHandle_t levelChangeScreenTaskHandle;
QueueHandle_t levelChange_queue;

struct changeScreenData myChangeScreenData;

void levelChangeScreenInit()
{
    levelChange_queue = xQueueCreate(1, sizeof(struct changeScreenData));
    xTaskCreate(levelChangeScreenDraw, "levelChangeScreenDraw", 2000, NULL, 3, &levelChangeScreenTaskHandle);
    vTaskSuspend(levelChangeScreenTaskHandle);
    
}

void levelChangeScreenEnter()
{
    
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, 0);
    xSemaphoreGive(g->mutex);
    vTaskResume(levelChangeScreenTaskHandle);
    xTaskNotifyGive(levelChangeScreenTaskHandle);
    xQueueReceive(levelChange_queue, &myChangeScreenData, 0);
}

void levelChangeScreenExit()
{
    GDisplay *g = gdispGetDisplay(0);
    xSemaphoreTake(g->mutex, portMAX_DELAY);
    vTaskSuspend(levelChangeScreenTaskHandle);
}

void levelChangeScreenDraw(void* data)
{
    uint8_t resetBool = 0;
    char str[25];
    uint32_t lastTime;
    uint32_t continueDelayTime = 1500;//in ms

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            if(resetBool == 0)
            {
                lastTime = xTaskGetTickCount();
                resetBool = 1;
            }
            gdispClear(Black);
            sprintf(str, myChangeScreenData.Title);
            gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font32)/2), 40, str, font32, White);
            sprintf(str, myChangeScreenData.Subtext);
            gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font24)/2), 90, str, font24, White);
            if(myChangeScreenData.showCountdown)
            {
                sprintf(str, "start in: %i", ((myChangeScreenData.msWaitingTime - (xTaskGetTickCount() - lastTime))/1000)+1);
                gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font20)/2), 120, str, font20, White);
            }

            if((xTaskGetTickCount() - lastTime) >= myChangeScreenData.msWaitingTime)
            {
                resetBool = 0;
                gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font16)/2), DISPLAY_SIZE_Y/2 + 50, str, font16, White);
                xQueueSend(state_queue, &myChangeScreenData.nextState, 0);
            }

        }
    }
    
}