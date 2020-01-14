#include "includes.h"

void levelChangeScreenInit();
void levelChangeScreenEnter();
void levelChangeScreenExit();
void levelChangeScreenDraw(void* data);

struct changeScreenData
{
    char text[20];
    int msWaitingTime;
    uint8_t showCountdown;
};
