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

    char str[100];
    uint32_t lastTime = xTaskGetTickCount();
    uint32_t continueDelayTime = 2000; //in ms

    while (1)
    {
        if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE)
        {
            gdispClear(Black);
            // drawUfo(&myufo, Grey);
            // updateUfo(&myufo);
            sprintf(str, "Level 1");
            gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font32)/2), 180, str, font32, White);

            if((xTaskGetTickCount() - lastTime) >= continueDelayTime)
            {
                gdispDrawString((DISPLAY_SIZE_X/2) - (gdispGetStringWidth(str, font16)/2), DISPLAY_SIZE_Y/2 + 50, str, font16, White);
                xQueueSend(state_queue, &gameStateId, 0);
            }

        }
    }
    
}