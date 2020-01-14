#include "includes.h"

void levelChangeScreenInit();
void levelChangeScreenEnter();
void levelChangeScreenExit();
void levelChangeScreenDraw(void* data);

extern QueueHandle_t levelChangeQueue;

struct changeScreenData
{
    char text[20];
    int msWaitingTime;
    uint8_t showCountdown;
};
