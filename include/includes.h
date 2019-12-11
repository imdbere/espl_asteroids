/**
 * Main include file.
 *
 * @author: Jonathan Müller-Boruttau, Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 **/
#pragma once

#include "stdio.h"

/* Board includes */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f429i_discovery_lcd.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* uGFX inclues. */
#include "gfx.h"

/* convenience functions and init includes */
#include "ESPL_functions.h"

#define DISPLAY_SIZE_X  320
#define DISPLAY_SIZE_Y  240
#define STATIC_STACK_SIZE 100

#define M_PI 3.14159265358979323846

#define max(a,b) \
	 ({ __typeof__ (a) _a = (a); \
			 __typeof__ (b) _b = (b); \
		 _a > _b ? _a : _b; })
		 
extern SemaphoreHandle_t DrawReady;
extern font_t font1;
extern font_t font12;
extern font_t font16;
extern font_t font20;
extern font_t font24;
extern font_t font32;
extern font_t fontLN;

struct pointf
{
	float x;
	float y;
};

typedef struct pointf pointf;