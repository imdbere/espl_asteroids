#include <stdlib.h>
#include <stdio.h>
#include "includes.h"

#define STATE_MACHINE_STATE_QUEUE_LENGTH 10
#define STATE_MACHINE_INTERVAL 10

typedef struct system_state {
	unsigned int _ID;

	void *data;

	void (*init)(void *);
	void (*enter)(void *);
	void (*run)(void *);
	void (*exit)(void *);

	struct system_state *next;

	unsigned char _initialized : 1;
} system_state_t;

struct state_machine {
	system_state_t head;

	unsigned int _state_count;

	system_state_t *current;
	system_state_t *next;

	unsigned char _initialized : 1;
} sm = { 0 };

QueueHandle_t state_queue = NULL;

unsigned int addState(void (*init)(void *), void (*enter)(void *),
		      /*void (*run)(void *),*/ void (*exit)(void *), void *data)
{
	system_state_t *iterator;

	for (iterator = &sm.head; iterator->next; iterator = iterator->next)
		;

	iterator->next = (system_state_t *)calloc(1, sizeof(system_state_t));
	iterator->next->init = init;
	iterator->next->enter = enter;
	//iterator->next->run = run;
	iterator->next->exit = exit;
	iterator->next->data = data;

	return (iterator->next->_ID = ++sm._state_count);
}


system_state_t __attribute__((optimize("O0"))) *findState(unsigned int ID)
{
	system_state_t *iterator;

	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	return iterator->next;
}

void deleteState(unsigned int ID)
{
	system_state_t *iterator, *delete;

	for (iterator = &sm.head; iterator->next && (iterator->next->_ID != ID);
	     iterator = iterator->next)
		;

	if (iterator->next)
		if (iterator->next->_ID) {
			delete = iterator->next;

			if (!iterator->next->next)
				iterator->next = NULL;
			else
				iterator->next = delete->next;

			free(delete);
		}
}

unsigned char smInit(void)
{
	system_state_t *iterator;

	state_queue = xQueueCreate(STATE_MACHINE_STATE_QUEUE_LENGTH,
				   sizeof(unsigned int));

	if (!state_queue) {
		fprintf(stderr, "State queue creation failed\n");
		//exit(EXIT_FAILURE);
	}

	for (iterator = &sm.head; iterator; iterator = iterator->next)
		if (iterator->init)
			(iterator->init)(iterator->data);

	return 0;
}

void initStateMachine()
{
	if (!(sm._initialized++))
		if (!smInit() && !(sm.current = sm.next = sm.head.next)) {
			fprintf(stderr, "No states\n");
			return;
			//exit(EXIT_FAILURE);
		}
		(sm.current->enter)(sm.current->data);
}

void statesHandlerTask(void* params)
{
	unsigned char state_in;
	TickType_t prev_wake_time;

	while (1) {
		while (xQueueReceive(state_queue, &state_in, portMAX_DELAY) != pdTRUE);

		sm.next = findState(state_in);
		if (sm.current != sm.next) {
			if (sm.current->exit)
				(sm.current->exit)(sm.current->data);

			if (sm.next->enter)
				(sm.next->enter)(sm.next->data);

			sm.current = sm.next;
		}
	}
}
