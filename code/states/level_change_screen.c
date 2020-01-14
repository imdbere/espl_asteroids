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
QueueHandle_t levelChangeQueue;

void levelChangeScreenInit()
{
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
    char str[100];
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
            sprintf(str, "Level 1");
            gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font32)/2), 40, str, font32, White);

            if((xTaskGetTickCount() - lastTime) >= continueDelayTime)
            {
                resetBool = 0;
                gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font16)/2), DISPLAY_SIZE_Y/2 + 50, str, font16, White);
                xQueueSend(state_queue, &gameStateId, 0);
            }

        }
    }
    
}