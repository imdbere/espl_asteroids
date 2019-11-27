#include "includes.h"

struct coord {
	uint8_t x;
	uint8_t y;
};

struct button {
	unsigned char previousState;
	unsigned char risingEdge;
	unsigned char fallingEdge;

	int pressCounter;
};

struct buttons {
    struct coord joystick;
    struct button A;
    struct button B;
    struct button C;
    struct button D;
    struct button E;
    struct button K;
};

extern QueueHandle_t ButtonQueue;
void initInputTask();