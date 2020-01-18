#include "input.h"
#include "stdlib.h"

QueueHandle_t ButtonQueue;
void checkInputs(void *params);

void initInputTask()
{
    ButtonQueue = xQueueCreate(1, sizeof(struct buttons));
    xTaskCreate(checkInputs, "checkButtons", 200, NULL, 4, NULL);
}

void updateButton(struct button *button, unsigned char currentState)
{
	(*button).risingEdge = 0;
	(*button).fallingEdge = 0;
	if (currentState && !(*button).previousState)
	{
		(*button).risingEdge = 1;
		(*button).pressCounter++;
	}
	else if (!currentState && (*button).previousState)
	{
		(*button).fallingEdge = 1;
	}
	(*button).previousState = currentState;
}

/**
 * This task polls the joystick and buttons every 20 ticks
 */
void checkInputs(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	struct buttons buttons = {{0}};
	const TickType_t PollingRate = 20;

	while (TRUE)
	{
		int joystickX = (int)(ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 128 ;
		buttons.joystick.x = abs(joystickX) > 10 ? joystickX : 0;

		int joystickY = (int)128 - (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
		buttons.joystick.y = abs(joystickY) > 10 ? joystickY : 0;

		// Edge detection
		updateButton(&buttons.A, !GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A));
		updateButton(&buttons.B, !GPIO_ReadInputDataBit(ESPL_Register_Button_B, ESPL_Pin_Button_B));
		updateButton(&buttons.C, !GPIO_ReadInputDataBit(ESPL_Register_Button_C, ESPL_Pin_Button_C));
		updateButton(&buttons.D, !GPIO_ReadInputDataBit(ESPL_Register_Button_D, ESPL_Pin_Button_D));
		updateButton(&buttons.E, !GPIO_ReadInputDataBit(ESPL_Register_Button_E, ESPL_Pin_Button_E));
		updateButton(&buttons.K, !GPIO_ReadInputDataBit(ESPL_Register_Button_K, ESPL_Pin_Button_K));

		xQueueSend(ButtonQueue, &buttons, 100);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, PollingRate);
	}
}
