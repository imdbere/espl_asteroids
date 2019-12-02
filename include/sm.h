#ifndef __SM_H__
#define __SM_H__
#include "includes.h"

extern QueueHandle_t state_queue;

unsigned int addState(void (*init)(void *), void (*enter)(void *),
		      /*void (*run)(void *),*/ void (*exit)(void *), void *data);
void statesHandlerTask(void* params);
void initStateMachine();

#endif