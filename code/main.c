#include "includes.h"
#include "sm.h"
#include "states/mainMenu.h"
#include "states/game.h"
#include "input.h"

#define STATE_COUNT 3

#define STATE_EX_TWO 0
#define STATE_EX_THREE 1
#define STATE_EX_FOUR 2

#define NEXT_TASK 1
#define PREV_TASK 2

font_t font1; // Load font for ugfx
font_t font32;
//Function prototypes
void frameSwapTask(void *params);

SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing

// Task handles, used for task control
TaskHandle_t frameSwapHandle;


int main(void)
{
	// Initialize Board functions and graphics
	unsigned int _mainMenuId;

	ESPL_SystemInit();

	font1 = gdispOpenFont("DejaVuSans24*");
	font32 = gdispOpenFont("DejaVuSans32*");

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 100, NULL, 4, &frameSwapHandle);
	xTaskCreate(statesHandlerTask, "statesHandlerTask", 200, NULL, 3, NULL);

	//addState(gameInit, gameEnter, gameRun, gameExit, NULL);
	_mainMenuId = addState(mainMenuInit, mainMenuEnter, mainMenuRun, mainMenuExit, NULL);
	
	//xQueueSend(state_queue, &_mainMenuId, 100);
	
	initInputTask();
	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

	

}

/*
 * Frame swapping happens in the background, seperate to all other system tasks.
 * This way it can be guarenteed that the 50fps requirement of the system
 * can be met.
 */
void frameSwapTask(void *params)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1)
	{
		// Draw next frame
		xSemaphoreGive(DrawReady);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// Swap buffers
		ESPL_DrawLayer();

		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
	}
}