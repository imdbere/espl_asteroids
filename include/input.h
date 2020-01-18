#include "includes.h"

struct coord {
	int x;
	int y;
};

struct button {
	unsigned char previousState;
	unsigned char risingEdge;
	unsigned char fallingEdge;
	int pressCounter;
    long lastDebounceTime; 
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