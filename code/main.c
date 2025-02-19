#include "includes.h"
#include "sm.h"
#include "states/main_menu.h"
#include "states/game.h"
#include "states/level_change_screen.h"
#include "input.h"
#include "uart.h"

font_t font1;
font_t font12;
font_t font16;
font_t font20;
font_t font24; 
font_t font32;
font_t fontLN;

//Function prototypes
void frameSwapTask(void *params);

SemaphoreHandle_t ESPL_DisplayReady;
SemaphoreHandle_t DrawReady; // After swapping buffer calll drawing

// Task handles, used for task control
TaskHandle_t frameSwapHandle;


int mainMenuStateId;
int gameStateId;
int levelChangeScreenId;

int main(void)
{
	// Initialize Board functions and graphics

	ESPL_SystemInit();

	font1 = gdispOpenFont("");
	font12 = gdispOpenFont("DejaVuSans12*");
	font16 = gdispOpenFont("DejaVuSans16*");
	font20 = gdispOpenFont("DejaVuSans20*");
	font24 = gdispOpenFont("DejaVuSans24*");
	font32 = gdispOpenFont("DejaVuSans32*");
	fontLN = gdispOpenFont("LargeNumbers");

	ESPL_DisplayReady = xSemaphoreCreateBinary();
	DrawReady = xSemaphoreCreateBinary();

	// Initializes Tasks with their respective priority
	// Core tasks
	xTaskCreate(frameSwapTask, "frameSwapper", 100, NULL, 4, &frameSwapHandle);
	xTaskCreate(statesHandlerTask, "statesHandlerTask", 200, NULL, 3, NULL);

	mainMenuStateId = addState(mainMenuInit, mainMenuEnter, mainMenuExit, NULL);
	gameStateId = addState(gameInit, gameEnter, gameExit, NULL);
	levelChangeScreenId = addState(levelChangeScreenInit, levelChangeScreenEnter, levelChangeScreenExit, NULL);
	initStateMachine();

	initInputTask();
	initUartQueues();
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